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

#include "core/utils/vpss_helper.h"

#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_kit.h"
#include "service/cvi_tdl_service.h"
#include "cvi_rtsp/rtsp.h"
#include <blt/parse/argparse_v2.h>
#include "classes.h"

#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static std::atomic<bool> g_running{true};
static std::atomic<bool> g_need_idr{true};

static void sig_handler(int sig)
{
    (void)sig;
    g_running = false;
}

static void rtsp_on_connect(const char* ip, void* arg)
{
    (void)arg;
    std::cout << "[RTSP] Client connected: " << (ip ? ip : "unknown") << "\n";
    g_need_idr = true;
}

static void rtsp_on_disconnect(const char* ip, void* arg)
{
    (void)arg;
    std::cout << "[RTSP] Client disconnected: " << (ip ? ip : "unknown") << "\n";
}

static void rtsp_on_play(int references, void* arg)
{
    (void)references;
    (void)arg;
    g_need_idr = true;
}

// ---------------------------------------------------------------------------
// Model registry: every supported vision model in one table.
// Add a new model = add a row here (plus labels in classes.h if desired).
// ---------------------------------------------------------------------------
struct ModelSpec
{
    const char* name;
    CVI_TDL_SUPPORTED_MODEL_E tdl_model;
    const char* default_path;
    const char* const* labels; // nullptr = use SDK's own info[i].name
    int label_count;
    int algo_cls;              // detection algo class count (0 = n/a)
};

static const ModelSpec MODEL_REGISTRY[] = {
    {"det", CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION,
     "/root/yolov8n_det_coco80_640_640_INT8_cv181x.cvimodel", coco_classes, 80, 80},
    {"fhp", CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION,
     "/root/yolov8n_det_face_head_person_pet_384_640_INT8_cv181x.cvimodel", fhp_classes, 4, 4},
    {"seg", CVI_TDL_SUPPORTED_MODEL_YOLOV8_SEG,
     "/root/yolov8n_seg_coco80_640_640_INT8_cv181x.cvimodel", coco_classes, 80, 80},
    {"pv", CVI_TDL_SUPPORTED_MODEL_PERSON_VEHICLE_DETECTION,
     "/root/yolov8n_det_person_vehicle_384_640_INT8_cv181x.cvimodel", pv_classes, 7, 7},
    {"pvlite", CVI_TDL_SUPPORTED_MODEL_PERSON_VEHICLE_DETECTION,
     "/root/yolov8n_det_person_vehicle_mv2_035_384_640_INT8_cv181x.cvimodel", pv_classes, 7, 7},
    {"face", CVI_TDL_SUPPORTED_MODEL_SCRFDFACE,
     "/root/scrfd_det_face_432_768_INT8_cv181x.cvimodel", nullptr, 0, 0},
};

static const ModelSpec* find_model(const std::string& key)
{
    for (const auto& m : MODEL_REGISTRY)
    {
        if (key == m.name) return &m;
    }
    return nullptr;
}

// YOLOv8 (det & seg) preprocessing per SDK sample: 1/255 normalize, RGB planar.
static CVI_S32 setup_yolov8_params(const cvitdl_handle_t handle, const ModelSpec& spec,
                                   float threshold, float nms_threshold)
{
    InputPreParam pre = CVI_TDL_GetPreParam(handle, spec.tdl_model);
    for (int i = 0; i < 3; i++)
    {
        pre.factor[i] = 0.003922f;
        pre.mean[i] = 0.0f;
    }
    pre.format = PIXEL_FORMAT_RGB_888_PLANAR;
    // Letterbox + center-rescale: without these the bbox rescaler gets
    // rescale_type=0 (RESCALE_UNKNOWN) and prints "Unsupported rescaling
    // method 0" once per detection box.
    pre.keep_aspect_ratio = true;
    pre.rescale_type = RESCALE_CENTER;
    pre.resize_method = VPSS_SCALE_COEF_BICUBIC;
    CVI_S32 ret = CVI_TDL_SetPreParam(handle, spec.tdl_model, pre);
    if (ret != CVI_SUCCESS)
    {
        std::cerr << "[TDL] SetPreParam failed: 0x" << std::hex << ret << "\n";
        return ret;
    }

    cvtdl_det_algo_param_t algo = CVI_TDL_GetDetectionAlgoParam(handle, spec.tdl_model);
    algo.cls = spec.algo_cls;
    ret = CVI_TDL_SetDetectionAlgoParam(handle, spec.tdl_model, algo);
    if (ret != CVI_SUCCESS)
    {
        std::cerr << "[TDL] SetDetectionAlgoParam failed: 0x" << std::hex << ret << "\n";
        return ret;
    }

    CVI_TDL_SetModelThreshold(handle, spec.tdl_model, threshold);
    CVI_TDL_SetModelNmsThreshold(handle, spec.tdl_model, nms_threshold);
    return CVI_SUCCESS;
}

// ---------------------------------------------------------------------------
// Line-crossing counters + async HTTP event reporter.
// ---------------------------------------------------------------------------
struct CrossingCounters
{
    std::atomic<uint64_t> person{0};
    std::atomic<uint64_t> vehicle{0};
};

static CrossingCounters g_counts;

// Count a track when its center crosses the virtual line. Uses previous-frame
// center (stored per id) so the crossing direction is well-defined and a track
// hovering on the line can only fire once per crossing.
struct TrackHistory
{
    std::unordered_map<uint64_t, std::pair<float, float>> last_center;
    uint64_t last_prune = 0;
};

static void update_crossings(TrackHistory& hist, const cvtdl_object_t& obj, uint32_t frame_h,
                             uint32_t frame_w, float line_pos, bool vertical)
{
    for (uint32_t i = 0; i < obj.size; i++)
    {
        if (obj.info[i].unique_id == 0) continue; // untracked
        float cx = (obj.info[i].bbox.x1 + obj.info[i].bbox.x2) * 0.5f;
        float cy = (obj.info[i].bbox.y1 + obj.info[i].bbox.y2) * 0.5f;
        auto it = hist.last_center.find(obj.info[i].unique_id);
        if (it != hist.last_center.end())
        {
            // compare the coordinate perpendicular to the line
            float prev = vertical ? it->second.first : it->second.second;
            float curr = vertical ? cx : cy;
            bool crossed = (prev < line_pos && curr >= line_pos) ||
                           (prev >= line_pos && curr < line_pos);
            if (crossed)
            {
                int cls = obj.info[i].classes;
                if (cls == 4)
                {
                    g_counts.person++;
                    std::cout << "[COUNT] person #" << obj.info[i].unique_id
                              << " total=" << g_counts.person.load() << "\n";
                }
                else
                {
                    g_counts.vehicle++;
                    std::cout << "[COUNT] vehicle(" << cls << ") #" << obj.info[i].unique_id
                              << " total=" << g_counts.vehicle.load() << "\n";
                }
            }
        }
        hist.last_center[obj.info[i].unique_id] = {cx, cy};
    }
    // prune dead tracks periodically (ids not seen this frame)
    if (frame_h > 0 && g_counts.person.load() >= 0) // cheap guard; prune below
    {
        if (hist.last_center.size() > 256 && frame_w > 0)
        {
            hist.last_prune++;
            if (hist.last_prune > 900) // ~30s at 30fps
            {
                hist.last_center.clear(); // ids will re-add on next sight
                hist.last_prune = 0;
            }
        }
    }
}

// Minimal async HTTP reporter: a background thread POSTs counter deltas as
// JSON whenever signalled. Socket + write(); no external deps.
struct HttpReporter
{
    std::string host;
    std::string port;
    std::string path;
    int interval_s = 10;
    std::atomic<bool> run{false};
    std::thread worker;
    std::mutex mtx;
    std::condition_variable cv;

    void start(std::string h, std::string pth, std::string prt, int itv)
    {
        host = std::move(h);
        path = std::move(pth);
        port = std::move(prt);
        interval_s = itv;
        run = true;
        worker = std::thread([this] { this->loop(); });
    }

    void stop()
    {
        run = false;
        cv.notify_all();
        if (worker.joinable()) worker.join();
    }

    void loop()
    {
        uint64_t last_p = 0, last_v = 0;
        while (run)
        {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait_for(lk, std::chrono::seconds(interval_s));
            lk.unlock();
            if (!run) break;

            uint64_t p = g_counts.person.load();
            uint64_t v = g_counts.vehicle.load();
            if (p == last_p && v == last_v) continue;

            char body[256];
            snprintf(body, sizeof(body),
                     "{\"device\":\"milkv-duo\",\"person\":%llu,\"vehicle\":%llu,"
                     "\"person_delta\":%llu,\"vehicle_delta\":%llu}",
                     (unsigned long long)p, (unsigned long long)v,
                     (unsigned long long)(p - last_p), (unsigned long long)(v - last_v));
            last_p = p;
            last_v = v;

            if (!post(body)) std::cerr << "[HTTP] post failed\n";
        }
    }

    bool post(const char* body)
    {
        bool ok = false;
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0 || !res) return false;
        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd >= 0)
        {
            if (connect(fd, res->ai_addr, res->ai_addrlen) == 0)
            {
                char req[512];
                int n = snprintf(req, sizeof(req),
                                 "POST %s HTTP/1.1\r\nHost: %s:%s\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Content-Length: %zu\r\nConnection: close\r\n\r\n%s",
                                 path.c_str(), host.c_str(), port.c_str(), strlen(body), body);
                ssize_t sent = 0;
                while (sent < n)
                {
                    ssize_t w = write(fd, req + sent, n - sent);
                    if (w <= 0) break;
                    sent += w;
                }
                ok = (sent == n);
                // drain a little of the response so close() isn't RST-y
                char buf[256];
                read(fd, buf, sizeof(buf));
            }
            close(fd);
        }
        freeaddrinfo(res);
        return ok;
    }
};

static HttpReporter g_reporter;

struct V4L2Buffer
{
    void* start = nullptr;
    size_t length = 0;
};

struct CameraConfig
{
    std::string device = "/dev/video0";
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t fps = 30;
    uint32_t pixelformat = V4L2_PIX_FMT_MJPEG;
};

static int init_v4l2_camera(const CameraConfig& cfg, int& cam_fd, std::vector<V4L2Buffer>& buffers,
                            uint32_t& negotiated_fmt)
{
    cam_fd = open(cfg.device.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (cam_fd < 0)
    {
        std::cerr << "[V4L2] Failed to open camera device: " << cfg.device << " (" << strerror(errno) << ")\n";
        return -1;
    }

    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        std::cerr << "[V4L2] VIDIOC_QUERYCAP failed: " << strerror(errno) << "\n";
        close(cam_fd);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        std::cerr << "[V4L2] Device " << cfg.device << " does not support video capture.\n";
        close(cam_fd);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        std::cerr << "[V4L2] Device " << cfg.device << " does not support streaming i/o.\n";
        close(cam_fd);
        return -1;
    }

    uint32_t preferred_fmts[] = {cfg.pixelformat, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_H264, V4L2_PIX_FMT_YUYV};
    bool fmt_set = false;
    struct v4l2_format fmt;

    for (uint32_t f : preferred_fmts)
    {
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = cfg.width;
        fmt.fmt.pix.height = cfg.height;
        fmt.fmt.pix.pixelformat = f;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

        if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) == 0)
        {
            negotiated_fmt = fmt.fmt.pix.pixelformat;
            fmt_set = true;
            char fourcc[5] = {0};
            memcpy(fourcc, &negotiated_fmt, 4);
            std::cout << "[V4L2] Successfully configured format: " << fourcc
                << " (" << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << ")\n";
            break;
        }
    }

    if (!fmt_set)
    {
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

    if (ioctl(cam_fd, VIDIOC_REQBUFS, &req) < 0 || req.count < 2)
    {
        std::cerr << "[V4L2] Insufficient buffer memory on " << cfg.device << "\n";
        close(cam_fd);
        return -1;
    }

    buffers.resize(req.count);
    for (uint32_t i = 0; i < req.count; ++i)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(cam_fd, VIDIOC_QUERYBUF, &buf) < 0)
        {
            std::cerr << "[V4L2] VIDIOC_QUERYBUF failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED)
        {
            std::cerr << "[V4L2] mmap failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }

        if (ioctl(cam_fd, VIDIOC_QBUF, &buf) < 0)
        {
            std::cerr << "[V4L2] VIDIOC_QBUF failed for buffer " << i << "\n";
            close(cam_fd);
            return -1;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(cam_fd, VIDIOC_STREAMON, &type) < 0)
    {
        std::cerr << "[V4L2] VIDIOC_STREAMON failed: " << strerror(errno) << "\n";
        close(cam_fd);
        return -1;
    }

    std::cout << "[V4L2] Streaming started from " << cfg.device << "\n";
    return 0;
}

static void stop_v4l2_camera(int cam_fd, std::vector<V4L2Buffer>& buffers)
{
    if (cam_fd >= 0)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(cam_fd, VIDIOC_STREAMOFF, &type);
        for (auto& b : buffers)
        {
            if (b.start && b.start != MAP_FAILED)
            {
                munmap(b.start, b.length);
            }
        }
        buffers.clear();
        close(cam_fd);
    }
}

static std::string parse_endpoint_name(const std::string& endpoint, int& port)
{
    std::string ep = endpoint;
    const std::string prefix = "rtsp://";
    if (ep.rfind(prefix, 0) == 0)
    {
        std::string rest = ep.substr(prefix.length());
        size_t slash_pos = rest.find('/');
        if (slash_pos != std::string::npos)
        {
            std::string host_port = rest.substr(0, slash_pos);
            ep = rest.substr(slash_pos + 1);
            size_t colon_pos = host_port.find(':');
            if (colon_pos != std::string::npos)
            {
                try
                {
                    port = std::stoi(host_port.substr(colon_pos + 1));
                }
                catch (...)
                {
                }
            }
        }
        else
        {
            ep = "live";
        }
    }
    while (!ep.empty() && ep.front() == '/')
    {
        ep.erase(0, 1);
    }
    if (ep.empty())
    {
        ep = "live";
    }
    return ep;
}

// ---------------------------------------------------------------------------
// Face-chain mode: SCRFD detection -> attribute/liveness second stages.
// Second-stage models consume the face meta from the detector, so they are
// configured separately from the single-frame MODEL_REGISTRY above.
// ---------------------------------------------------------------------------
struct FaceChainConfig
{
    bool enabled = false;
    bool with_attr = false;     // cls_attribute_gender_age_glass_emotion
    bool with_liveness = false; // cls_rgbliveness
    std::string fd_path = "/root/scrfd_det_face_432_768_INT8_cv181x.cvimodel";
    std::string attr_path = "/root/cls_attribute_gender_age_glass_emotion_112_112_INT8_cv181x.cvimodel";
    std::string live_path = "/root/cls_rgbliveness_256_256_INT8_cv181x.cvimodel";
};

static const char* emotion_name(cvtdl_face_emotion_e e)
{
    switch (e)
    {
        case EMOTION_HAPPY: return "happy";
        case EMOTION_SURPRISE: return "surprise";
        case EMOTION_FEAR: return "fear";
        case EMOTION_DISGUST: return "disgust";
        case EMOTION_SAD: return "sad";
        case EMOTION_ANGER: return "anger";
        case EMOTION_NEUTRAL: return "neutral";
        default: return "unknown";
    }
}

static void draw_faces(cvitdl_service_handle_t service, const cvtdl_face_t& faces,
                       VIDEO_FRAME_INFO_S* frame, const FaceChainConfig& fc)
{
    cvtdl_service_brush_t brush{};
    brush.color.r = 0.2f; brush.color.g = 1.0f; brush.color.b = 0.6f;
    brush.size = 2;
    CVI_TDL_Service_FaceDrawRect(service, &faces, frame, false, brush);

    char label[160];
    for (uint32_t i = 0; i < faces.size; i++)
    {
        const cvtdl_face_info_t& f = faces.info[i];
        if (fc.with_attr)
        {
            snprintf(label, sizeof(label), "%s %s %.0fy %.0f%%",
                     (f.gender == GENDER_FEMALE) ? "F" : "M",
                     emotion_name(f.emotion), f.age * 100.0f,
                     (f.glass >= 0.5f ? 100.0f : 0.0f));
            CVI_TDL_Service_ObjectWriteText(label,
                static_cast<int>(f.bbox.x1), static_cast<int>(f.bbox.y1) - 26, frame,
                1.0f, 0.9f, 0.2f);
        }
        if (fc.with_liveness)
        {
            snprintf(label, sizeof(label), "live %.2f", f.liveness_score);
            CVI_TDL_Service_ObjectWriteText(label,
                static_cast<int>(f.bbox.x1), static_cast<int>(f.bbox.y2) + 4, frame,
                f.liveness_score >= 0.8f ? 0.2f : 1.0f, 0.4f, 0.2f);
        }
    }
}

// ---------------------------------------------------------------------------
// Per-model inference + drawing
// ---------------------------------------------------------------------------
static uint32_t color_u32(int idx)
{
    static const uint32_t palette[] = {
        0x00FF40, 0xFF4040, 0x4040FF, 0xFFFF40, 0x40FFFF, 0xFF40FF, 0xFFFFFF, 0xFF9040};
    return palette[idx % 8];
}

static void draw_objects(cvitdl_service_handle_t service, const cvtdl_object_t& obj,
                         VIDEO_FRAME_INFO_S* frame, const ModelSpec& spec, bool draw_masks)
{
    cvtdl_service_brush_t brush{};
    brush.color.r = 0.0f;
    brush.color.g = 1.0f;
    brush.color.b = 0.0f;
    brush.size = 2;

    CVI_TDL_Service_ObjectDrawRect(service, &obj, frame, false, brush);

    // Class labels (SDK may not fill info[i].name for yolov8; use our tables).
    char label[160];
    int y = 4;
    for (uint32_t i = 0; i < obj.size; i++)
    {
        const char* cls_name = "object";
        if (spec.labels && obj.info[i].classes < spec.label_count)
        {
            cls_name = spec.labels[obj.info[i].classes];
        }
        else if (obj.info[i].name[0] != '\0')
        {
            cls_name = obj.info[i].name;
        }
        if (obj.info[i].unique_id > 0)
        {
            snprintf(label, sizeof(label), "#%lu %s %.0f%%",
                     (unsigned long)obj.info[i].unique_id, cls_name,
                     obj.info[i].bbox.score * 100.0f);
        }
        else
        {
            snprintf(label, sizeof(label), "%s %.0f%%", cls_name,
                     obj.info[i].bbox.score * 100.0f);
        }

        int bx = static_cast<int>(obj.info[i].bbox.x1);
        int by = static_cast<int>(obj.info[i].bbox.y1) + y;
        CVI_TDL_Service_ObjectWriteText(label, bx, by, frame, 1.0f, 1.0f, 0.0f);

        // Seg masks: Set_MaskOutlinePoint gives interleaved x,y pairs; DrawPolygon
        // wants separate x/y arrays -> de-interleave. Draw every 2nd point so the
        // overlay stays cheap at 720p.
        if (draw_masks && obj.info[i].mask_properity &&
            obj.info[i].mask_properity->mask_point_size >= 3)
        {
            const uint32_t n = obj.info[i].mask_properity->mask_point_size;
            std::vector<float> xs(n), ys(n);
            for (uint32_t j = 0; j < n; ++j)
            {
                xs[j] = obj.info[i].mask_properity->mask_point[2 * j];
                ys[j] = obj.info[i].mask_properity->mask_point[2 * j + 1];
            }
            uint32_t stride = (n > 60) ? 2 : 1;
            std::vector<float> px, py;
            px.reserve(n / stride + 1);
            py.reserve(n / stride + 1);
            for (uint32_t j = 0; j < n; j += stride)
            {
                px.push_back(xs[j]);
                py.push_back(ys[j]);
            }
            cvtdl_pts_t poly{};
            poly.x = px.data();
            poly.y = py.data();
            poly.size = static_cast<uint32_t>(px.size());
            poly.score = 1.0f;

            uint32_t c = color_u32(obj.info[i].classes);
            cvtdl_service_brush_t mbrush{};
            mbrush.color.r = ((c >> 16) & 0xFF) / 255.0f;
            mbrush.color.g = ((c >> 8) & 0xFF) / 255.0f;
            mbrush.color.b = (c & 0xFF) / 255.0f;
            mbrush.size = 2;
            CVI_TDL_Service_DrawPolygon(service, frame, &poly, mbrush);
        }
    }
}

static void run_classification(const cvitdl_handle_t handle, VIDEO_FRAME_INFO_S* frame,
                               const ModelSpec& spec, float threshold)
{
    cvtdl_class_meta_t cls_meta;
    std::memset(&cls_meta, 0, sizeof(cls_meta));
    CVI_S32 ret = CVI_TDL_Image_Classification(handle, frame, &cls_meta);
    if (ret != CVI_SUCCESS)
    {
        static uint32_t err_count = 0;
        if (++err_count % 30 == 1)
        {
            std::cerr << "[TDL] classification failed: 0x" << std::hex << ret << "\n";
        }
        return;
    }
    char label[160];
    int y = 4;
    for (int i = 0; i < 5; i++)
    {
        const char* name = "class";
        if (cls_meta.cls[i] >= 0 && cls_meta.cls[i] < 1000)
        {
            name = imagenet_classes[cls_meta.cls[i]];
        }
        snprintf(label, sizeof(label), "Top %d: %s (%.2f%%)", i + 1, name,
                 cls_meta.score[i] * 100.0f);
        std::cout << label << "\n";
        if (cls_meta.score[i] >= threshold || i == 0)
        {
            CVI_TDL_Service_ObjectWriteText(label, 30, 30 + y, frame, 0.0f, 1.0f, 0.0f);
            y += 30;
        }
    }
    CVI_TDL_FreeClassMeta(&cls_meta);
}

int main(int argc, char** argv)
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGQUIT, sig_handler);

    blt::argparse::argument_parser_t parser{};
    parser.add_flag("--camera", "-c").set_default(std::string("/dev/video0")).as_type<std::string>().set_dest("camera");
    parser.add_flag("--mode", "-M").set_default(std::string("seg")).as_type<std::string>().set_dest("mode");
    parser.add_flag("--model", "-m").set_default(std::string("")).as_type<std::string>().set_dest("model");
    parser.add_flag("--endpoint", "-e").set_default(std::string("live")).as_type<std::string>().set_dest("endpoint");
    parser.add_flag("--port", "-p").set_default(8554).as_type<int>().set_dest("port");
    parser.add_flag("--width", "-w").set_default(1280).as_type<int>().set_dest("width");
    parser.add_flag("--height", "-h").set_default(720).as_type<int>().set_dest("height");
    parser.add_flag("--fps", "-f").set_default(30).as_type<int>().set_dest("fps");
    parser.add_flag("--threshold", "-t").set_default(0.5f).as_type<float>().set_dest("threshold");
    parser.add_flag("--nms", "-n").set_default(0.5f).as_type<float>().set_dest("nms");
    parser.add_flag("--bitrate", "-b").set_default(2000).as_type<int>().set_dest("bitrate");
    parser.add_flag("--no-masks").make_flag().set_dest("nomasks");
    parser.add_flag("--stride", "-s").set_default(1).as_type<int>().set_dest("stride");
    parser.add_flag("--track").make_flag().set_dest("track");
    parser.add_flag("--count").make_flag().set_dest("count");
    parser.add_flag("--line", "-L").set_default(0.5f).as_type<float>().set_dest("line");
    parser.add_flag("--horizontal").make_flag().set_dest("horiz");
    parser.add_flag("--report").set_default(std::string("")).as_type<std::string>().set_dest("report");
    parser.add_flag("--report-interval", "-R").set_default(10).as_type<int>().set_dest("reportinterval");
    parser.add_flag("--face-attr").make_flag().set_dest("faceattr");
    parser.add_flag("--face-live").make_flag().set_dest("facelive");
    parser.add_flag("--face-fd").set_default(std::string("")).as_type<std::string>().set_dest("facefd");
    parser.add_flag("--face-attr-model").set_default(std::string("")).as_type<std::string>().set_dest("faceattrmodel");
    parser.add_flag("--face-live-model").set_default(std::string("")).as_type<std::string>().set_dest("facelivemodel");
    parser.add_flag("--face-tiny").set_default(false).as_type<bool>().set_dest("facetiny");
    parser.with_help();

    auto args = parser.parse(argc, const_cast<const char**>(argv));

    std::string camera_dev = args.get<std::string>("camera");
    std::string mode = args.get<std::string>("mode");
    std::string model_override = args.get<std::string>("model");
    std::string endpoint_arg = args.get<std::string>("endpoint");
    int rtsp_port = args.get<int>("port");
    int width = args.get<int>("width");
    int height = args.get<int>("height");
    int fps = args.get<int>("fps");
    float conf_threshold = args.get<float>("threshold");
    float nms_threshold = args.get<float>("nms");
    int bitrate_kbps = args.get<int>("bitrate");
    bool draw_masks = !args.get<bool>("nomasks");
    int infer_stride = args.get<int>("stride");
    if (infer_stride < 1) infer_stride = 1;
    bool use_tracker = args.get<bool>("track");
    bool use_counter = args.get<bool>("count");
    if (use_counter && !use_tracker)
    {
        std::cerr << "[COUNT] --count requires --track (counting is per-track)\n";
        return 1;
    }
    float line_frac = args.get<float>("line");
    if (line_frac <= 0.0f || line_frac >= 1.0f) line_frac = 0.6f;
    // Default gate is VERTICAL (camera perpendicular to the route, objects
    // transit left/right). --horizontal restores the across-the-lane gate.
    bool line_vertical = !args.get<bool>("horiz");
    std::string report_url = args.get<std::string>("report");
    int report_interval = args.get<int>("reportinterval");
    if (report_interval < 1) report_interval = 1;
    // split http://host:port/path
    std::string rep_host, rep_port = "80", rep_path = "/";
    if (!report_url.empty())
    {
        std::string u = report_url;
        const std::string scheme = "http://";
        if (u.rfind(scheme, 0) == 0) u = u.substr(scheme.size());
        size_t slash = u.find('/');
        std::string hostport = (slash == std::string::npos) ? u : u.substr(0, slash);
        rep_path = (slash == std::string::npos) ? "/" : u.substr(slash);
        size_t colon = hostport.find(':');
        rep_host = (colon == std::string::npos) ? hostport : hostport.substr(0, colon);
        if (colon != std::string::npos) rep_port = hostport.substr(colon + 1);
        if (rep_host.empty())
        {
            std::cerr << "[HTTP] bad --report url\n";
            return 1;
        }
    }

    FaceChainConfig face_cfg;
    face_cfg.enabled = (mode == "face");
    if (args.get<bool>("facetiny"))
        face_cfg.attr_path = "/root/cls_attribute_gender_age_glass_emotion_tiny_112_112_INT8_cv181x.cvimodel";
    face_cfg.with_attr = args.get<bool>("faceattr");
    face_cfg.with_liveness = args.get<bool>("facelive");
    if (!args.get<std::string>("facefd").empty()) face_cfg.fd_path = args.get<std::string>("facefd");
    if (!args.get<std::string>("faceattrmodel").empty()) face_cfg.attr_path = args.get<std::string>("faceattrmodel");
    if (!args.get<std::string>("facelivemodel").empty()) face_cfg.live_path = args.get<std::string>("facelivemodel");

    const ModelSpec* spec = find_model(mode);
    if (!spec)
    {
        std::cerr << "[ERROR] Unknown mode '" << mode << "'. Valid: ";
        for (const auto& m : MODEL_REGISTRY) std::cerr << m.name << " ";
        std::cerr << "\n";
        return 1;
    }
    std::string model_path = model_override.empty() ? spec->default_path : model_override;

    std::string session_name = parse_endpoint_name(endpoint_arg, rtsp_port);

    std::cout << "========================================\n"
        << " Milk-V Duo S Hardware Accelerated Streamer [" << spec->name << "]\n"
        << " Camera:    " << camera_dev << "\n"
        << " Model:     " << model_path << "\n"
        << " Endpoint:  rtsp://<ip>:" << rtsp_port << "/" << session_name << "\n"
        << " Resolution:" << width << "x" << height << " @" << fps << "fps\n"
        << " Threshold: " << conf_threshold << "  NMS: " << nms_threshold << "\n"
        << " Bitrate:   " << bitrate_kbps << " kbps\n"
        << " Stride:    inference every " << infer_stride << " frame(s)\n"
        << (use_counter ? std::string(" Counting:  ") + (line_vertical ? "vertical" : "horizontal") +
                              " line at " + std::to_string(line_frac) + " of frame " +
                              (line_vertical ? "width" : "height")
                              + (report_url.empty() ? "" : ", report -> " + rep_host + ":" + rep_port + rep_path) + "\n"
                        : std::string(""))
        << (face_cfg.enabled ? " FaceChain: attr=" + std::string(face_cfg.with_attr ? "on" : "off") +
                                  " liveness=" + std::string(face_cfg.with_liveness ? "on" : "off") + "\n"
                             : std::string(""))
        << "========================================\n";

    // 1. Initialize V4L2 Camera
    int cam_fd = -1;
    std::vector<V4L2Buffer> v4l2_buffers;
    uint32_t cam_fmt = V4L2_PIX_FMT_MJPEG;
    CameraConfig cam_cfg{camera_dev, (uint32_t)width, (uint32_t)height, (uint32_t)fps, V4L2_PIX_FMT_MJPEG};

    if (init_v4l2_camera(cam_cfg, cam_fd, v4l2_buffers, cam_fmt) != 0)
    {
        std::cerr << "[ERROR] Could not initialize camera.\n";
        return 1;
    }

    // 2. Initialize Middleware (VB + SYS)
    auto s32Ret = MMF_INIT_HELPER2(width, height, PIXEL_FORMAT_YUV_PLANAR_420, 8,
                              width, height, PIXEL_FORMAT_YUV_PLANAR_420, 4);
    if (s32Ret != CVI_SUCCESS)
    {
        std::cerr << "[CVI] MMF_INIT_HELPER2 failed: 0x" << std::hex << s32Ret << "\n";
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }

    // 3. Initialize VDEC
    VDEC_CHN vdec_chn = 0;
    VDEC_CHN_ATTR_S stVdecAttr;
    memset(&stVdecAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    stVdecAttr.enType = (cam_fmt == V4L2_PIX_FMT_H264) ? PT_H264 : PT_JPEG;
    stVdecAttr.enMode = VIDEO_MODE_FRAME;
    stVdecAttr.u32PicWidth = width;
    stVdecAttr.u32PicHeight = height;
    stVdecAttr.u32StreamBufSize = ALIGN(width * height, 0x4000);
    stVdecAttr.u32FrameBufCnt = 4;

    if (stVdecAttr.enType == PT_JPEG)
    {
        stVdecAttr.u32FrameBufSize = VDEC_GetPicBufferSize(PT_JPEG, width, height, PIXEL_FORMAT_YUV_PLANAR_420,
                                                           DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    }

    s32Ret = CVI_VDEC_CreateChn(vdec_chn, &stVdecAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        std::cerr << "[CVI VDEC] CreateChn failed: 0x" << std::hex << s32Ret << "\n";
    }

    VDEC_CHN_PARAM_S stVdecChnParam;
    memset(&stVdecChnParam, 0, sizeof(stVdecChnParam));
    CVI_VDEC_GetChnParam(vdec_chn, &stVdecChnParam);
    stVdecChnParam.enType = stVdecAttr.enType;
    stVdecChnParam.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
    stVdecChnParam.u32DisplayFrameNum = 2;
    CVI_VDEC_SetChnParam(vdec_chn, &stVdecChnParam);
    CVI_VDEC_StartRecvStream(vdec_chn);

    // 4. Initialize VENC
    VENC_CHN venc_chn = 0;
    VENC_CHN_ATTR_S stVencChnAttr;
    memset(&stVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    stVencChnAttr.stVencAttr.enType = PT_H264;
    stVencChnAttr.stVencAttr.u32MaxPicWidth = width;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = height;
    stVencChnAttr.stVencAttr.u32PicWidth = width;
    stVencChnAttr.stVencAttr.u32PicHeight = height;
    stVencChnAttr.stVencAttr.u32BufSize = width * height;
    stVencChnAttr.stVencAttr.u32Profile = 0; // Baseline
    stVencChnAttr.stVencAttr.bByFrame = CVI_TRUE;

    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    stVencChnAttr.stRcAttr.stH264Cbr.u32Gop = (fps > 0 && fps <= 30) ? fps : 30;
    stVencChnAttr.stRcAttr.stH264Cbr.u32StatTime = 1;
    stVencChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = fps;
    stVencChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = fps;
    stVencChnAttr.stRcAttr.stH264Cbr.u32BitRate = bitrate_kbps;

    stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 2;

    s32Ret = CVI_VENC_CreateChn(venc_chn, &stVencChnAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        std::cerr << "[CVI VENC] CreateChn failed: 0x" << std::hex << s32Ret << "\n";
    }

    VENC_H264_ENTROPY_S h264Entropy = {0};
    h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CAVLC;
    h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CAVLC;
    CVI_VENC_SetH264Entropy(venc_chn, &h264Entropy);

    VENC_H264_TRANS_S h264Trans = {0};
    if (CVI_VENC_GetH264Trans(venc_chn, &h264Trans) == CVI_SUCCESS)
    {
        h264Trans.chroma_qp_index_offset = 0;
        CVI_VENC_SetH264Trans(venc_chn, &h264Trans);
    }

    VENC_H264_VUI_S h264Vui = {0};
    if (CVI_VENC_GetH264Vui(venc_chn, &h264Vui) == CVI_SUCCESS)
    {
        h264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 1;
        h264Vui.stVuiAspectRatio.aspect_ratio_idc = 1;
        h264Vui.stVuiTimeInfo.timing_info_present_flag = 1;
        h264Vui.stVuiTimeInfo.fixed_frame_rate_flag = 0;
        h264Vui.stVuiTimeInfo.num_units_in_tick = 1;
        h264Vui.stVuiTimeInfo.time_scale = (fps > 0 ? fps : 30) * 2;
        CVI_VENC_SetH264Vui(venc_chn, &h264Vui);
    }

    VENC_RECV_PIC_PARAM_S stRecvParam;
    memset(&stRecvParam, 0, sizeof(stRecvParam));
    stRecvParam.s32RecvPicNum = -1;
    CVI_VENC_StartRecvFrame(venc_chn, &stRecvParam);

    // 5. Initialize CVI RTSP Server
    CVI_RTSP_CONFIG rtsp_cfg;
    memset(&rtsp_cfg, 0, sizeof(rtsp_cfg));
    rtsp_cfg.port = rtsp_port;

    CVI_RTSP_CTX* rtsp_ctx = nullptr;
    if (CVI_RTSP_Create(&rtsp_ctx, &rtsp_cfg) < 0)
    {
        std::cerr << "[CVI RTSP] Failed to create RTSP context on port " << rtsp_port << "\n";
    }

    CVI_RTSP_SESSION* rtsp_session = nullptr;
    CVI_RTSP_SESSION_ATTR session_attr;
    memset(&session_attr, 0, sizeof(session_attr));
    session_attr.video.codec = RTSP_VIDEO_H264;
    session_attr.video.bitrate = bitrate_kbps;
    session_attr.video.play = rtsp_on_play;
    session_attr.video.playArg = nullptr;
    snprintf(session_attr.name, sizeof(session_attr.name), "%s", session_name.c_str());

    CVI_RTSP_CreateSession(rtsp_ctx, &session_attr, &rtsp_session);

    CVI_RTSP_STATE_LISTENER rtsp_listener;
    memset(&rtsp_listener, 0, sizeof(rtsp_listener));
    rtsp_listener.onConnect = rtsp_on_connect;
    rtsp_listener.onDisconnect = rtsp_on_disconnect;
    rtsp_listener.argConn = rtsp_ctx;
    CVI_RTSP_SetListener(rtsp_ctx, &rtsp_listener);

    if (CVI_RTSP_Start(rtsp_ctx) < 0)
    {
        std::cerr << "[CVI RTSP] Failed to start RTSP server.\n";
    }
    else
    {
        std::cout << "[CVI RTSP] RTSP Server listening on rtsp://0.0.0.0:" << rtsp_port << "/" << session_name << "\n";
    }

    // 6. Initialize CVI TDL
    cvitdl_handle_t tdl_handle = nullptr;
    s32Ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (s32Ret != CVI_SUCCESS)
    {
        std::cerr << "[CVI TDL] CreateHandle failed: 0x" << std::hex << s32Ret << "\n";
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }

    // Params must be set before OpenModel for yolov8 family.
    if (spec->tdl_model == CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION ||
        spec->tdl_model == CVI_TDL_SUPPORTED_MODEL_YOLOV8_SEG)
    {
        if (setup_yolov8_params(tdl_handle, *spec, conf_threshold, nms_threshold) != CVI_SUCCESS)
        {
            stop_v4l2_camera(cam_fd, v4l2_buffers);
            return 1;
        }
    }

    s32Ret = CVI_TDL_OpenModel(tdl_handle, spec->tdl_model, model_path.c_str());
    if (s32Ret != CVI_SUCCESS)
    {
        std::cerr << "[CVI TDL] OpenModel failed for " << model_path << ": 0x" << std::hex << s32Ret << "\n";
        stop_v4l2_camera(cam_fd, v4l2_buffers);
        return 1;
    }
    std::cout << "[CVI TDL] Model loaded: " << model_path << "\n";

    if (face_cfg.enabled)
    {
        // SCRFD itself was opened via the registry row above.
        if (face_cfg.with_attr)
        {
            s32Ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEATTRIBUTE_CLS, face_cfg.attr_path.c_str());
            if (s32Ret != CVI_SUCCESS)
            {
                std::cerr << "[CVI TDL] FaceAttr open failed: 0x" << std::hex << s32Ret << "\n";
                stop_v4l2_camera(cam_fd, v4l2_buffers);
                return 1;
            }
        }
        if (face_cfg.with_liveness)
        {
            s32Ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_IRLIVENESS, face_cfg.live_path.c_str());
            if (s32Ret != CVI_SUCCESS)
            {
                std::cerr << "[CVI TDL] Liveness open failed: 0x" << std::hex << s32Ret << "\n";
                stop_v4l2_camera(cam_fd, v4l2_buffers);
                return 1;
            }
        }
        std::cout << "[CVI TDL] Face chain ready (attr=" << face_cfg.with_attr
                  << " live=" << face_cfg.with_liveness << ")\n";
    }

    // DeepSORT/ByteTrack: Kalman + IoU matching for consistent object IDs.
    // use_reid=false -> no re-ID model needed (feartrack/OSNet not wired).
    if (use_tracker)
    {
        s32Ret = CVI_TDL_DeepSORT_Init(tdl_handle, true);
        if (s32Ret != CVI_SUCCESS)
        {
            std::cerr << "[CVI TDL] DeepSORT init failed: 0x" << std::hex << s32Ret << "\n";
            stop_v4l2_camera(cam_fd, v4l2_buffers);
            return 1;
        }
        cvtdl_deepsort_config_t ds_conf;
        CVI_TDL_DeepSORT_GetDefaultConfig(&ds_conf);
        ds_conf.ktracker_conf.accreditation_threshold = 3; // frames before an ID is confirmed
        CVI_TDL_DeepSORT_SetConfig(tdl_handle, &ds_conf, -1, true);
        std::cout << "[CVI TDL] DeepSORT tracker ready (reid off)\n";
    }

    if (!report_url.empty())
    {
        g_reporter.start(rep_host, rep_path, rep_port, report_interval);
        std::cout << "[HTTP] reporter -> " << rep_host << ":" << rep_port << rep_path
                  << " every " << report_interval << "s\n";
    }
    TrackHistory track_hist;
    float line_pos = line_frac * (float)(line_vertical ? width : height);

    cvitdl_service_handle_t service_handle = nullptr;
    CVI_TDL_Service_CreateHandle(&service_handle, tdl_handle);

    std::cout << "[MAIN] Processing loop started. Press Ctrl+C to terminate.\n";

    uint64_t frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    std::vector<uint8_t> s_sps;
    std::vector<uint8_t> s_pps;

    while (g_running)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        struct timeval tv = {.tv_sec = 2, .tv_usec = 0};

        int r = select(cam_fd + 1, &fds, nullptr, nullptr, &tv);
        if (r <= 0)
        {
            if (r < 0 && errno == EINTR) continue;
            std::cerr << "[V4L2] Camera frame timeout\n";
            continue;
        }

        struct v4l2_buffer v4l2_buf;
        memset(&v4l2_buf, 0, sizeof(v4l2_buf));
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(cam_fd, VIDIOC_DQBUF, &v4l2_buf) < 0)
        {
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
        ioctl(cam_fd, VIDIOC_QBUF, &v4l2_buf);

        VIDEO_FRAME_INFO_S stFrame;
        memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = CVI_VDEC_GetFrame(vdec_chn, &stFrame, -1);
        if (s32Ret == CVI_SUCCESS)
        {
            // ---- Inference + overlay, dispatched by mode ----
            // With --stride N the TPU runs every Nth frame; skipped frames
            // stream clean (no overlay) at full pipeline speed.
            if (infer_stride > 1 && (frame_count % infer_stride) != 0)
            {
                // inference skipped this frame
            }
            else if (spec->tdl_model == CVI_TDL_SUPPORTED_MODEL_IMAGE_CLASSIFICATION)
            {
                run_classification(tdl_handle, &stFrame, *spec, conf_threshold);
            }
            else if (face_cfg.enabled)
            {
                cvtdl_face_t faces;
                std::memset(&faces, 0, sizeof(faces));
                CVI_S32 fret = CVI_TDL_FaceDetection(tdl_handle, &stFrame,
                                                     CVI_TDL_SUPPORTED_MODEL_SCRFDFACE, &faces);
                if (fret == CVI_SUCCESS && faces.size > 0)
                {
                    if (face_cfg.with_attr)
                    {
                        CVI_TDL_FaceAttribute_cls(tdl_handle, &stFrame, &faces);
                    }
                    if (face_cfg.with_liveness)
                    {
                        CVI_TDL_IrLiveness(tdl_handle, &stFrame, &faces);
                    }
                    draw_faces(service_handle, faces, &stFrame, face_cfg);
                }
                CVI_TDL_FreeFace(&faces);
            }
            else
            {
                cvtdl_object_t obj_meta;
                std::memset(&obj_meta, 0, sizeof(obj_meta));
                cvtdl_tracker_t trk_meta;
                std::memset(&trk_meta, 0, sizeof(trk_meta));
                CVI_S32 iret = (spec->tdl_model == CVI_TDL_SUPPORTED_MODEL_YOLOV8_SEG)
                    ? CVI_TDL_YoloV8_Seg(tdl_handle, &stFrame, &obj_meta)
                    : CVI_TDL_Detection(tdl_handle, &stFrame, spec->tdl_model, &obj_meta);

                if (iret == CVI_SUCCESS)
                {
                    if (spec->tdl_model == CVI_TDL_SUPPORTED_MODEL_YOLOV8_SEG)
                    {
                        CVI_TDL_Set_MaskOutlinePoint(&stFrame, &obj_meta);
                    }
                    if (use_tracker)
                    {
                        // Assigns persistent IDs: obj_meta.info[i].unique_id and
                        // trk_meta.info[i].id stay consistent across frames.
                        CVI_TDL_DeepSORT_Obj(tdl_handle, &obj_meta, &trk_meta, false);
                    }
                    if (use_counter)
                    {
                        update_crossings(track_hist, obj_meta, (uint32_t)height, (uint32_t)width,
                                         line_pos, line_vertical);
                        // draw the virtual counting line so viewers see the gate
                        cvtdl_pts_t line_pts{};
                        float lx[2], ly[2];
                        if (line_vertical)
                        {
                            lx[0] = line_pos; lx[1] = line_pos;
                            ly[0] = 0.0f;     ly[1] = (float)height;
                        }
                        else
                        {
                            lx[0] = 0.0f;     lx[1] = (float)width;
                            ly[0] = line_pos; ly[1] = line_pos;
                        }
                        line_pts.x = lx;
                        line_pts.y = ly;
                        line_pts.size = 2;
                        line_pts.score = 1.0f;
                        cvtdl_service_brush_t lbrush{};
                        lbrush.color.r = 1.0f; lbrush.color.g = 0.0f; lbrush.color.b = 1.0f;
                        lbrush.size = 2;
                        CVI_TDL_Service_DrawPolygon(service_handle, &stFrame, &line_pts, lbrush);
                        char cbuf[96];
                        snprintf(cbuf, sizeof(cbuf), "P:%llu V:%llu",
                                 (unsigned long long)g_counts.person.load(),
                                 (unsigned long long)g_counts.vehicle.load());
                        CVI_TDL_Service_ObjectWriteText(cbuf,
                                                        line_vertical ? (int)line_pos + 8 : 10,
                                                        line_vertical ? 40 : (int)line_pos - 30,
                                                        &stFrame, 1.0f, 0.0f, 1.0f);
                    }
                    draw_objects(service_handle, obj_meta, &stFrame, *spec, draw_masks);
                }
                else
                {
                    static uint32_t det_err = 0;
                    if (++det_err % 30 == 1)
                    {
                        std::cerr << "[TDL] detection/seg failed: 0x" << std::hex << iret << "\n";
                    }
                }
                CVI_TDL_FreeTracker(&trk_meta);
                CVI_TDL_Free(&obj_meta);
            }

            // IDR on client connect
            if (g_need_idr.exchange(false))
            {
                CVI_VENC_RequestIDR(venc_chn, CVI_TRUE);
            }

            // Encode + stream (unchanged pipeline tail)
            s32Ret = CVI_VENC_SendFrame(venc_chn, &stFrame, -1);
            if (s32Ret != CVI_SUCCESS)
            {
                std::cerr << "[CVI VENC] SendFrame failed: 0x" << std::hex << s32Ret << "\n";
            }

            CVI_VDEC_ReleaseFrame(vdec_chn, &stFrame);

            VENC_CHN_STATUS_S stStat;
            if (CVI_VENC_QueryStatus(venc_chn, &stStat) == CVI_SUCCESS && stStat.u32CurPacks > 0)
            {
                VENC_STREAM_S stStream;
                memset(&stStream, 0, sizeof(VENC_STREAM_S));
                stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                if (stStream.pstPack)
                {
                    if (CVI_VENC_GetStream(venc_chn, &stStream, 10000) == CVI_SUCCESS)
                    {
                        if (rtsp_ctx && rtsp_session && rtsp_session->video)
                        {
                            bool has_sps = false;
                            bool has_pps = false;
                            bool has_idr = false;

                            for (uint32_t i = 0; i < stStream.u32PackCount; i++)
                            {
                                VENC_PACK_S* p = &stStream.pstPack[i];
                                uint32_t len = p->u32Len - p->u32Offset;
                                uint8_t* ptr = p->pu8Addr + p->u32Offset;
                                if (p->DataType.enH264EType == H264E_NALU_SPS)
                                {
                                    has_sps = true;
                                    s_sps.assign(ptr, ptr + len);
                                }
                                else if (p->DataType.enH264EType == H264E_NALU_PPS)
                                {
                                    has_pps = true;
                                    s_pps.assign(ptr, ptr + len);
                                }
                                else if (p->DataType.enH264EType == H264E_NALU_IDRSLICE)
                                {
                                    has_idr = true;
                                }
                            }

                            CVI_RTSP_DATA rtsp_data = {0};
                            uint32_t bIdx = 0;

                            if (has_idr && !has_sps && !s_sps.empty() && bIdx < CVI_RTSP_DATA_MAX_BLOCK)
                            {
                                rtsp_data.dataPtr[bIdx] = s_sps.data();
                                rtsp_data.dataLen[bIdx] = (uint32_t)s_sps.size();
                                bIdx++;
                            }
                            if (has_idr && !has_pps && !s_pps.empty() && bIdx < CVI_RTSP_DATA_MAX_BLOCK)
                            {
                                rtsp_data.dataPtr[bIdx] = s_pps.data();
                                rtsp_data.dataLen[bIdx] = (uint32_t)s_pps.size();
                                bIdx++;
                            }

                            for (uint32_t i = 0; i < stStream.u32PackCount && bIdx < CVI_RTSP_DATA_MAX_BLOCK; i++)
                            {
                                rtsp_data.dataPtr[bIdx] = stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset;
                                rtsp_data.dataLen[bIdx] = stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
                                bIdx++;
                            }
                            rtsp_data.blockCnt = bIdx;
                            CVI_RTSP_WriteFrame(rtsp_ctx, rtsp_session->video, &rtsp_data);
                        }
                        CVI_VENC_ReleaseStream(venc_chn, &stStream);
                    }
                    free(stStream.pstPack);
                }
            }

            frame_count++;
            if (frame_count % 150 == 0)
            {
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() /
                    1000.0;
                std::cout << "[FPS] Processed " << frame_count << " frames in " << elapsed << "s ("
                    << (frame_count / elapsed) << " fps)\n";
            }
        }
        else if (s32Ret == CVI_ERR_VDEC_ERR_INVALID_RET)
        {
            // display queue momentarily empty; keep going
        }
        else
        {
            static uint32_t vdec_err_count = 0;
            if (++vdec_err_count % 30 == 1)
            {
                std::cerr << "[CVI VDEC] GetFrame waiting/error: 0x" << std::hex << s32Ret << "\n";
            }
        }
    }

    std::cout << "[MAIN] Shutting down pipeline...\n";
    g_reporter.stop();

    if (rtsp_ctx)
    {
        CVI_RTSP_Stop(rtsp_ctx);
        if (rtsp_session)
        {
            CVI_RTSP_DestroySession(rtsp_ctx, rtsp_session);
        }
        CVI_RTSP_Destroy(&rtsp_ctx);
    }

    CVI_VENC_StopRecvFrame(venc_chn);
    CVI_VENC_ResetChn(venc_chn);
    CVI_VENC_DestroyChn(venc_chn);

    CVI_VDEC_StopRecvStream(vdec_chn);
    CVI_VDEC_ResetChn(vdec_chn);
    CVI_VDEC_DestroyChn(vdec_chn);

    // NOTE: DeepSORT state lives in the tdl handle; no separate teardown API
    // (DestroyHandle frees it). CVI_TDL_DeepSORT_CleanCounter resets IDs only.
    if (service_handle)
    {
        CVI_TDL_Service_DestroyHandle(service_handle);
    }
    if (tdl_handle)
    {
        CVI_TDL_DestroyHandle(tdl_handle);
    }

    stop_v4l2_camera(cam_fd, v4l2_buffers);

    CVI_SYS_Exit();
    CVI_VB_Exit();

    std::cout << "[MAIN] Pipeline shutdown complete.\n";
    return 0;
}
