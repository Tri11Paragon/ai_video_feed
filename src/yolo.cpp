#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/videodev2.h>

#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vdec.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_comm_vdec.h"
#include "cvi_comm_venc.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vb.h"

#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "service/cvi_tdl_service.h"
#include "cvi_rtsp/rtsp.h"
#include <blt/parse/argparse_v2.h>

static std::atomic<bool> g_running{true};

static void sig_handler(int sig)
{
    (void)sig;
    g_running = false;
}

static void rtsp_on_connect(const char *ip, void *arg)
{
    (void)arg;
    std::cout << "[RTSP] Client connected: " << (ip ? ip : "unknown") << "\n";
}

static void rtsp_on_disconnect(const char *ip, void *arg)
{
    (void)arg;
    std::cout << "[RTSP] Client disconnected: " << (ip ? ip : "unknown") << "\n";
}

struct V4L2Buffer {
    void *start = nullptr;
    size_t length = 0;
};

struct CameraConfig {
    std::string device = "/dev/video0";
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t fps = 30;
    uint32_t pixelformat = V4L2_PIX_FMT_MJPEG;
};

static int init_v4l2_camera(const CameraConfig &cfg, int &cam_fd, std::vector<V4L2Buffer> &buffers, uint32_t &negotiated_fmt)
{
    cam_fd = open(cfg.device.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (cam_fd < 0) {
        std::cerr << "[V4L2] Failed to open camera device: " << cfg.device << " (" << strerror(errno) << ")\n";
        return -1;
    }

    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        std::cerr << "[V4L2] VIDIOC_QUERYCAP failed: " << strerror(errno) << "\n";
        close(cam_fd);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        std::cerr << "[V4L2] Device " << cfg.device << " does not support video capture.\n";
        close(cam_fd);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        std::cerr << "[V4L2] Device " << cfg.device << " does not support streaming i/o.\n";
        close(cam_fd);
        return -1;
    }

    uint32_t preferred_fmts[] = {cfg.pixelformat, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_H264, V4L2_PIX_FMT_YUYV};
    bool fmt_set = false;
    struct v4l2_format fmt;

    for (uint32_t f : preferred_fmts) {
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = cfg.width;
        fmt.fmt.pix.height = cfg.height;
        fmt.fmt.pix.pixelformat = f;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

        if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) == 0) {
            negotiated_fmt = fmt.fmt.pix.pixelformat;
            fmt_set = true;
            char fourcc[5] = {0};
            memcpy(fourcc, &negotiated_fmt, 4);
            std::cout << "[V4L2] Successfully configured format: " << fourcc
                      << " (" << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << ")\n";
            break;
        }
    }

    if (!fmt_set) {
        std::cerr << "[V4L2] Failed to set video format on " << cfg.device << "\n";
        close(cam_fd);
        return -1;
    }

    // Set frame rate
    struct v4l2_streamparm streamparm;
    memset(&streamparm, 0, sizeof(streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = cfg.fps;
    ioctl(cam_fd, VIDIOC_S_PARM, &streamparm);

    // Request buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(cam_fd, VIDIOC_REQBUFS, &req) < 0 || req.count < 2) {
        std::cerr << "[V4L2] Insufficient buffer memory on " << cfg.device << "\n";
        close(cam_fd);
        return -1;
    }

    buffers.resize(req.count);
    for (uint32_t i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(cam_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            std::cerr << "[V4L2] VIDIOC_QUERYBUF failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED) {
            std::cerr << "[V4L2] mmap failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }

        if (ioctl(cam_fd, VIDIOC_QBUF, &buf) < 0) {
            std::cerr << "[V4L2] VIDIOC_QBUF failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(cam_fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "[V4L2] VIDIOC_STREAMON failed: " << strerror(errno) << "\n";
        close(cam_fd);
        return -1;
    }

    std::cout << "[V4L2] Streaming started from " << cfg.device << "\n";
    return 0;
}

static void stop_v4l2_camera(int cam_fd, std::vector<V4L2Buffer> &buffers)
{
    if (cam_fd >= 0) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(cam_fd, VIDIOC_STREAMOFF, &type);
        for (auto &b : buffers) {
            if (b.start && b.start != MAP_FAILED) {
                munmap(b.start, b.length);
            }
        }
        buffers.clear();
        close(cam_fd);
    }
}

static std::string parse_endpoint_name(const std::string &endpoint, int &port)
{
    std::string ep = endpoint;
    const std::string prefix = "rtsp://";
    if (ep.rfind(prefix, 0) == 0) {
        std::string rest = ep.substr(prefix.length());
        size_t slash_pos = rest.find('/');
        if (slash_pos != std::string::npos) {
            std::string host_port = rest.substr(0, slash_pos);
            ep = rest.substr(slash_pos + 1);
            size_t colon_pos = host_port.find(':');
            if (colon_pos != std::string::npos) {
                try {
                    port = std::stoi(host_port.substr(colon_pos + 1));
                } catch (...) {}
            }
        } else {
            ep = "live";
        }
    }
    while (!ep.empty() && ep.front() == '/') {
        ep.erase(0, 1);
    }
    if (ep.empty()) {
        ep = "live";
    }
    return ep;
}

int main(int argc, char** argv)
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGQUIT, sig_handler);

    blt::argparse::argument_parser_t parser{};
    parser.add_flag("--camera", "-c").set_default(std::string("/dev/video0")).as_type<std::string>().set_dest("camera");
    parser.add_flag("--model", "-m").set_default(std::string("/root/yolov8n_cv181x_int8_sym.cvimodel")).as_type<std::string>().set_dest("model");
    parser.add_flag("--endpoint", "-e").set_default(std::string("live")).as_type<std::string>().set_dest("endpoint");
    parser.add_flag("--port", "-p").set_default(8554).as_type<int>().set_dest("port");
    parser.add_flag("--width", "-w").set_default(1280).as_type<int>().set_dest("width");
    parser.add_flag("--height", "-h").set_default(720).as_type<int>().set_dest("height");
    parser.add_flag("--fps", "-f").set_default(30).as_type<int>().set_dest("fps");
    parser.add_flag("--threshold", "-t").set_default(0.5f).as_type<float>().set_dest("threshold");
    parser.add_flag("--nms").set_default(0.5f).as_type<float>().set_dest("nms");
    parser.add_flag("--bitrate", "-b").set_default(2000).as_type<int>().set_dest("bitrate");

    auto args = parser.parse(argc, const_cast<const char**>(argv));

    std::string camera_dev = args.get<std::string>("camera");
    std::string model_path = args.get<std::string>("model");
    std::string endpoint_arg = args.get<std::string>("endpoint");
    int rtsp_port = args.get<int>("port");
    int width = args.get<int>("width");
    int height = args.get<int>("height");
    int fps = args.get<int>("fps");
    float conf_threshold = args.get<float>("threshold");
    float nms_threshold = args.get<float>("nms");
    int bitrate_kbps = args.get<int>("bitrate");

    std::string session_name = parse_endpoint_name(endpoint_arg, rtsp_port);

    std::cout << "========================================\n"
              << " Milk-V Duo S Hardware Accelerated Streamer\n"
              << " Camera:    " << camera_dev << "\n"
              << " Model:     " << model_path << "\n"
              << " Endpoint:  rtsp://<ip>:" << rtsp_port << "/" << session_name << "\n"
              << " Resolution:" << width << "x" << height << " @" << fps << "fps\n"
              << " Threshold: " << conf_threshold << " (NMS: " << nms_threshold << ")\n"
              << " Bitrate:   " << bitrate_kbps << " kbps\n"
              << "========================================\n";

    // 1. Initialize V4L2 Camera
    int cam_fd = -1;
    std::vector<V4L2Buffer> v4l2_buffers;
    uint32_t cam_fmt = V4L2_PIX_FMT_MJPEG;
    CameraConfig cam_cfg{camera_dev, (uint32_t)width, (uint32_t)height, (uint32_t)fps, V4L2_PIX_FMT_MJPEG};

    if (init_v4l2_camera(cam_cfg, cam_fd, v4l2_buffers, cam_fmt) != 0) {
        std::cerr << "[ERROR] Could not initialize camera.\n";
        return 1;
    }

    // 2. Initialize Hardware VB Memory Pool & System
    CVI_SYS_Exit();
    CVI_VB_Exit();

    VB_CONFIG_S stVbConf;
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 2;
    stVbConf.astCommPool[0].u32BlkSize = COMMON_GetPicBufferSize(width, height, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u32BlkCnt = 8;
    stVbConf.astCommPool[1].u32BlkSize = COMMON_GetPicBufferSize(width, height, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u32BlkCnt = 4;

    CVI_S32 s32Ret = CVI_VB_SetConfig(&stVbConf);
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI] CVI_VB_SetConfig failed: 0x" << std::hex << s32Ret << "\n";
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI] CVI_VB_Init failed: 0x" << std::hex << s32Ret << "\n";
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }

    s32Ret = CVI_SYS_Init();
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI] CVI_SYS_Init failed: 0x" << std::hex << s32Ret << "\n";
        CVI_VB_Exit();
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }

    // 3. Initialize VDEC (Hardware Decoder) for camera stream (MJPEG / H264)
    VDEC_CHN vdec_chn = 0;
    VDEC_CHN_ATTR_S stVdecAttr;
    memset(&stVdecAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    stVdecAttr.enType = (cam_fmt == V4L2_PIX_FMT_H264) ? PT_H264 : PT_JPEG;
    stVdecAttr.enMode = VIDEO_MODE_FRAME;
    stVdecAttr.u32PicWidth = width;
    stVdecAttr.u32PicHeight = height;
    stVdecAttr.u32StreamBufSize = ALIGN(width * height, 0x4000);
    stVdecAttr.u32FrameBufCnt = 4;

    if (stVdecAttr.enType == PT_JPEG) {
        stVdecAttr.u32FrameBufSize = VDEC_GetPicBufferSize(PT_JPEG, width, height, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    }

    s32Ret = CVI_VDEC_CreateChn(vdec_chn, &stVdecAttr);
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI VDEC] CreateChn failed: 0x" << std::hex << s32Ret << "\n";
    }

    VDEC_CHN_PARAM_S stVdecChnParam;
    memset(&stVdecChnParam, 0, sizeof(stVdecChnParam));
    CVI_VDEC_GetChnParam(vdec_chn, &stVdecChnParam);
    stVdecChnParam.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
    stVdecChnParam.u32DisplayFrameNum = 2;
    CVI_VDEC_SetChnParam(vdec_chn, &stVdecChnParam);
    CVI_VDEC_StartRecvStream(vdec_chn);

    // 4. Initialize VENC (Hardware H.264 Encoder)
    VENC_CHN venc_chn = 0;
    VENC_CHN_ATTR_S stVencChnAttr;
    memset(&stVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    stVencChnAttr.stVencAttr.enType = PT_H264;
    stVencChnAttr.stVencAttr.u32MaxPicWidth = width;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = height;
    stVencChnAttr.stVencAttr.u32PicWidth = width;
    stVencChnAttr.stVencAttr.u32PicHeight = height;
    stVencChnAttr.stVencAttr.u32BufSize = width * height;
    stVencChnAttr.stVencAttr.u32Profile = 0; // Baseline/Main
    stVencChnAttr.stVencAttr.bByFrame = CVI_TRUE;

    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    stVencChnAttr.stRcAttr.stH264Cbr.u32Gop = fps * 2;
    stVencChnAttr.stRcAttr.stH264Cbr.u32StatTime = 1;
    stVencChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = fps;
    stVencChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = fps;
    stVencChnAttr.stRcAttr.stH264Cbr.u32BitRate = bitrate_kbps;

    stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 2;

    s32Ret = CVI_VENC_CreateChn(venc_chn, &stVencChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI VENC] CreateChn failed: 0x" << std::hex << s32Ret << "\n";
    }

    VENC_RECV_PIC_PARAM_S stRecvParam;
    memset(&stRecvParam, 0, sizeof(stRecvParam));
    stRecvParam.s32RecvPicNum = -1;
    CVI_VENC_StartRecvFrame(venc_chn, &stRecvParam);

    // 5. Initialize CVI RTSP Server
    CVI_RTSP_CONFIG rtsp_cfg;
    memset(&rtsp_cfg, 0, sizeof(rtsp_cfg));
    rtsp_cfg.port = rtsp_port;

    CVI_RTSP_CTX *rtsp_ctx = nullptr;
    if (CVI_RTSP_Create(&rtsp_ctx, &rtsp_cfg) < 0) {
        std::cerr << "[CVI RTSP] Failed to create RTSP context on port " << rtsp_port << "\n";
    }

    CVI_RTSP_SESSION *rtsp_session = nullptr;
    CVI_RTSP_SESSION_ATTR session_attr;
    memset(&session_attr, 0, sizeof(session_attr));
    session_attr.video.codec = RTSP_VIDEO_H264;
    snprintf(session_attr.name, sizeof(session_attr.name), "%s", session_name.c_str());

    CVI_RTSP_CreateSession(rtsp_ctx, &session_attr, &rtsp_session);

    CVI_RTSP_STATE_LISTENER rtsp_listener;
    memset(&rtsp_listener, 0, sizeof(rtsp_listener));
    rtsp_listener.onConnect = rtsp_on_connect;
    rtsp_listener.onDisconnect = rtsp_on_disconnect;
    rtsp_listener.argConn = rtsp_ctx;
    CVI_RTSP_SetListener(rtsp_ctx, &rtsp_listener);

    if (CVI_RTSP_Start(rtsp_ctx) < 0) {
        std::cerr << "[CVI RTSP] Failed to start RTSP server.\n";
    } else {
        std::cout << "[CVI RTSP] RTSP Server listening on rtsp://0.0.0.0:" << rtsp_port << "/" << session_name << "\n";
    }

    // 6. Initialize CVI TDL (YOLOv8 Model & Drawing Service)
    cvitdl_handle_t tdl_handle = nullptr;
    s32Ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI TDL] CreateHandle failed: 0x" << std::hex << s32Ret << "\n";
    }

    s32Ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION, model_path.c_str());
    if (s32Ret != CVI_SUCCESS) {
        std::cerr << "[CVI TDL] OpenModel failed for " << model_path << ": 0x" << std::hex << s32Ret << "\n";
        std::cerr << "[CVI TDL] Please ensure the YOLOv8 cvimodel exists and path is correct.\n";
    } else {
        std::cout << "[CVI TDL] Successfully loaded YOLOv8 model: " << model_path << "\n";
    }

    CVI_TDL_SetModelThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION, conf_threshold);
    CVI_TDL_SetModelNmsThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION, nms_threshold);

    cvitdl_service_handle_t service_handle = nullptr;
    CVI_TDL_Service_CreateHandle(&service_handle, tdl_handle);

    std::cout << "[MAIN] Processing loop started. Press Ctrl+C to terminate.\n";

    uint64_t frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (g_running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };

        int r = select(cam_fd + 1, &fds, nullptr, nullptr, &tv);
        if (r <= 0) {
            if (r < 0 && errno == EINTR) continue;
            std::cerr << "[V4L2] Camera frame timeout\n";
            continue;
        }

        struct v4l2_buffer v4l2_buf;
        memset(&v4l2_buf, 0, sizeof(v4l2_buf));
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(cam_fd, VIDIOC_DQBUF, &v4l2_buf) < 0) {
            if (errno == EAGAIN) continue;
            std::cerr << "[V4L2] VIDIOC_DQBUF error: " << strerror(errno) << "\n";
            break;
        }

        // Send compressed frame chunk to hardware VDEC
        VDEC_STREAM_S stVdecStream;
        memset(&stVdecStream, 0, sizeof(VDEC_STREAM_S));
        stVdecStream.pu8Addr = (CVI_U8*)v4l2_buffers[v4l2_buf.index].start;
        stVdecStream.u32Len = v4l2_buf.bytesused;
        stVdecStream.bEndOfFrame = CVI_TRUE;
        stVdecStream.bEndOfStream = CVI_FALSE;

        CVI_VDEC_SendStream(vdec_chn, &stVdecStream, 2000);

        // Re-queue V4L2 buffer immediately
        ioctl(cam_fd, VIDIOC_QBUF, &v4l2_buf);

        // Retrieve hardware decoded video frame
        VIDEO_FRAME_INFO_S stFrame;
        memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = CVI_VDEC_GetFrame(vdec_chn, &stFrame, 2000);
        if (s32Ret == CVI_SUCCESS) {
            // 1. Run YOLOv8 Object Detection on TPU
            cvtdl_object_t obj_meta = {0};
            if (tdl_handle) {
                s32Ret = CVI_TDL_YOLOV8_Detection(tdl_handle, &stFrame, &obj_meta);
                if (s32Ret == CVI_SUCCESS && obj_meta.size > 0 && service_handle) {
                    for (uint32_t i = 0; i < obj_meta.size; i++) {
                        char label[256];
                        snprintf(label, sizeof(label), "%s: %.2f", obj_meta.info[i].name, obj_meta.info[i].bbox.score);
                        strncpy(obj_meta.info[i].name, label, sizeof(obj_meta.info[i].name) - 1);
                    }
                    cvtdl_service_brush_t brush = CVI_TDL_Service_GetDefaultBrush();
                    brush.size = 2;
                    CVI_TDL_Service_ObjectDrawRect(service_handle, &obj_meta, &stFrame, true, brush);
                }
            }

            // 2. Hardware H.264 encode via VENC
            s32Ret = CVI_VENC_SendFrame(venc_chn, &stFrame, 20000);
            if (s32Ret != CVI_SUCCESS) {
                std::cerr << "[CVI VENC] SendFrame failed: 0x" << std::hex << s32Ret << "\n";
            }

            // Release decoded frame back to VDEC pool
            CVI_VDEC_ReleaseFrame(vdec_chn, &stFrame);
            CVI_TDL_Free(&obj_meta);

            // 3. Query VENC and stream encoded NAL units to RTSP
            VENC_CHN_STATUS_S stStat;
            if (CVI_VENC_QueryStatus(venc_chn, &stStat) == CVI_SUCCESS && stStat.u32CurPacks > 0) {
                VENC_STREAM_S stStream;
                memset(&stStream, 0, sizeof(VENC_STREAM_S));
                stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                if (stStream.pstPack) {
                    if (CVI_VENC_GetStream(venc_chn, &stStream, 10000) == CVI_SUCCESS) {
                        if (rtsp_ctx && rtsp_session && rtsp_session->video) {
                            CVI_RTSP_DATA rtsp_data = {0};
                            rtsp_data.blockCnt = stStream.u32PackCount;
                            for (unsigned int i = 0; i < stStream.u32PackCount; i++) {
                                rtsp_data.dataPtr[i] = stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset;
                                rtsp_data.dataLen[i] = stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
                            }
                            CVI_RTSP_WriteFrame(rtsp_ctx, rtsp_session->video, &rtsp_data);
                        }
                        CVI_VENC_ReleaseStream(venc_chn, &stStream);
                    }
                    free(stStream.pstPack);
                }
            }

            frame_count++;
            if (frame_count % 150 == 0) {
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() / 1000.0;
                std::cout << "[FPS] Processed " << frame_count << " frames in " << elapsed << "s ("
                          << (frame_count / elapsed) << " fps)\n";
            }
        }
    }

    std::cout << "[MAIN] Shutting down pipeline...\n";

    // Clean up RTSP
    if (rtsp_ctx) {
        CVI_RTSP_Stop(rtsp_ctx);
        if (rtsp_session) {
            CVI_RTSP_DestroySession(rtsp_ctx, rtsp_session);
        }
        CVI_RTSP_Destroy(&rtsp_ctx);
    }

    // Clean up VENC
    CVI_VENC_StopRecvFrame(venc_chn);
    CVI_VENC_ResetChn(venc_chn);
    CVI_VENC_DestroyChn(venc_chn);

    // Clean up VDEC
    CVI_VDEC_StopRecvStream(vdec_chn);
    CVI_VDEC_ResetChn(vdec_chn);
    CVI_VDEC_DestroyChn(vdec_chn);

    // Clean up TDL
    if (service_handle) {
        CVI_TDL_Service_DestroyHandle(service_handle);
    }
    if (tdl_handle) {
        CVI_TDL_DestroyHandle(tdl_handle);
    }

    // Clean up V4L2
    stop_v4l2_camera(cam_fd, v4l2_buffers);

    // Clean up CVI System & VB
    CVI_SYS_Exit();
    CVI_VB_Exit();

    std::cout << "[MAIN] Pipeline shutdown complete.\n";
    return 0;
}
