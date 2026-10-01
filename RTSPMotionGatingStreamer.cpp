#include <thread>
+// Motion gate (OpenCV)
+#include <opencv2/core.hpp>
+#include <opencv2/imgproc.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
+// For pixel conversion
+#include <libswscale/swscale.h>
}

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <utility>
+#include <chrono>
+#include <cmath>

// ... existing code ...

 struct Args {
     std::string input = "/dev/video0";
     std::string url;                   // rtsp://...
     std::string format = "";           // "h264" or "mjpeg" (optional hint)
     std::string size = "";             // e.g. "1280x720"
     std::string transport = "tcp";     // "tcp" or "udp"
     int fps = 0;                       // optional
     int max_delay_ms = 500;            // network max_delay
+    // Motion options
+    int motion_enable = 1;             // enable motion gate
+    double motion_thresh = 10.0;       // percentage of changed pixels (0-100)
+    int motion_cooldown_ms = 5000;     // continue streaming this long after last motion
+    int motion_scale_w = 160;          // downscale width for motion processing
+    int idle_send_fps = 0;             // 0 = send nothing while idle; otherwise sparse keepalive FPS
 };

 static Args parse_args(int argc, char** argv) {
     Args a;
     for (int i = 1; i < argc; ++i) {
         std::string k = argv[i];
         auto next = [&](std::string def = "") -> std::string {
             if (i + 1 < argc) return std::string(argv[++i]);
             return def;
         };
         if (k == "--input") a.input = next();
         else if (k == "--url") a.url = next();
         else if (k == "--format") a.format = next();       // "h264" or "mjpeg"
         else if (k == "--size") a.size = next();           // "WIDTHxHEIGHT"
         else if (k == "--fps") a.fps = std::atoi(next().c_str());
         else if (k == "--transport") a.transport = next(); // "tcp" or "udp"
         else if (k == "--max_delay_ms") a.max_delay_ms = std::atoi(next().c_str());
+        else if (k == "--motion_enable") a.motion_enable = std::atoi(next().c_str());
+        else if (k == "--motion_thresh") a.motion_thresh = std::atof(next().c_str());
+        else if (k == "--motion_cooldown_ms") a.motion_cooldown_ms = std::atoi(next().c_str());
+        else if (k == "--motion_scale_w") a.motion_scale_w = std::atoi(next().c_str());
+        else if (k == "--idle_send_fps") a.idle_send_fps = std::atoi(next().c_str());
         else if (k == "--help" || k == "-h") {
             std::cout <<
                 "Usage: rtsp_push --url rtsp://host:port/path [--input /dev/video0]\n"
                 "                 [--format h264|mjpeg] [--size 1280x720] [--fps 30]\n"
-                "                 [--transport tcp|udp] [--max_delay_ms 500]\n";
+                "                 [--transport tcp|udp] [--max_delay_ms 500]\n"
+                "Motion gating:\n"
+                "  --motion_enable 1            enable motion gating (0/1)\n"
+                "  --motion_thresh 10.0         % pixels changed to trigger motion\n"
+                "  --motion_cooldown_ms 5000    keep streaming this long after motion\n"
+                "  --motion_scale_w 160         downscale width for motion processing\n"
+                "  --idle_send_fps 0            send FPS while idle (0 = drop all)\n";
             std::exit(0);
         }
     }
     return a;
 }

// ... existing code ...
     // Some drivers need 'seekable' = 0 to avoid blocking
     av_dict_set(&in_opts, "seekable", "0", 0);
+    // Prefer wallclock-based timestamps from the demuxer (helps some drivers)
+    av_dict_set(&in_opts, "use_wallclock_as_timestamps", "1", 0);

     AVFormatContext* in_ctx = nullptr;
// ... existing code ...
     // Log input codec
     const AVCodecParameters* in_par = in_st->codecpar;
// ... existing code ...

+    // Initialize decoder for motion processing
+    const AVCodec* dec = avcodec_find_decoder(in_par->codec_id);
+    AVCodecContext* dec_ctx = nullptr;
+    if (!dec) {
+        log_ff("Decoder not found for motion processing");
+    } else {
+        dec_ctx = avcodec_alloc_context3(dec);
+        if (!dec_ctx) {
+            log_ff("avcodec_alloc_context3 failed for decoder");
+        } else {
+            if (avcodec_parameters_to_context(dec_ctx, in_par) < 0) {
+                log_ff("avcodec_parameters_to_context failed for decoder");
+                avcodec_free_context(&dec_ctx);
+                dec_ctx = nullptr;
+            } else if (avcodec_open2(dec_ctx, dec, nullptr) < 0) {
+                log_ff("avcodec_open2 failed for decoder");
+                avcodec_free_context(&dec_ctx);
+                dec_ctx = nullptr;
+            }
+        }
+    }
+
+    // Sws context to convert decoded frames to GRAY8
+    SwsContext* sws = nullptr;
+    int sws_w = 0, sws_h = 0, sws_src_fmt = AV_PIX_FMT_NONE;
+
     // Prepare RTSP output
     AVFormatContext* out_ctx = nullptr;
// ... existing code ...
     // Timebase: use input stream timebase if present, else default
-    out_st->time_base = in_st->time_base.num && in_st->time_base.den
-                        ? in_st->time_base
-                        : AVRational{1, 90000};
+    // Use 90k video clock for a stable RTP timeline
+    out_st->time_base = AVRational{1, 90000};

     // Print destination
     av_dump_format(out_ctx, 0, args.url.c_str(), 1);
// ... existing code ...
     std::cerr << "Streaming to " << args.url << " (Press Ctrl+C to stop)\n";

     // Main forwarding loop
     AVPacket pkt;
     av_init_packet(&pkt);
+
+    // Derive fps and per-frame duration (in 90k ticks)
+    AVRational fps_q = in_st->avg_frame_rate.num && in_st->avg_frame_rate.den
+                       ? in_st->avg_frame_rate
+                       : (in_st->r_frame_rate.num && in_st->r_frame_rate.den
+                          ? in_st->r_frame_rate
+                          : AVRational{args.fps > 0 ? args.fps : 30, 1});
+    double fps = av_q2d(fps_q);
+    if (fps < 1.0 || !std::isfinite(fps)) {
+        fps = (args.fps > 0) ? args.fps : 30.0;
+        fps_q = AVRational{static_cast<int>(fps + 0.5), 1};
+    }
+    int64_t frame_duration = static_cast<int64_t>(std::llround(90000.0 / fps));
+    if (frame_duration < 1) frame_duration = 1;
+
+    // Motion gate state
+    using clock = std::chrono::steady_clock;
+    auto last_motion = clock::time_point::min();
+    bool motion_active = false;
+    int frames_per_idle = (args.idle_send_fps > 0)
+                          ? std::max(1, static_cast<int>(std::lround(fps / args.idle_send_fps)))
+                          : INT32_MAX;
+    int64_t timeline = 0;       // increases per input frame
+    uint64_t frame_index = 0;   // increases per input frame
+
+    // Motion buffers
+    cv::Mat prev_gray_small, gray_small, diff, thresh;
+    int target_w = std::max(32, args.motion_scale_w);
+    int target_h = 0; // computed after first frame

     while (!g_stop) {
         ret = av_read_frame(in_ctx, &pkt);
         if (ret == AVERROR(EAGAIN)) {
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
             continue;
         } else if (ret < 0) {
             // End or error. Try a short wait and continue, or break
             log_ff("av_read_frame returned", ret);
             break;
         }

         if (pkt.stream_index != video_stream_index) {
             av_packet_unref(&pkt);
             continue;
         }
+
+        // Decode for motion (non-blocking loop)
+        if (dec_ctx && args.motion_enable) {
+            AVPacket pkt_copy = pkt; // avcodec_send_packet takes ownership only on success; safe to copy
+            if (avcodec_send_packet(dec_ctx, &pkt_copy) >= 0) {
+                AVFrame* frm = av_frame_alloc();
+                while (frm && avcodec_receive_frame(dec_ctx, frm) == 0) {
+                    // Prepare sws on first frame or if format changed
+                    if (!sws || sws_w != frm->width || sws_h != frm->height || sws_src_fmt != frm->format) {
+                        if (sws) sws_freeContext(sws);
+                        sws_w = frm->width; sws_h = frm->height; sws_src_fmt = frm->format;
+                        sws = sws_getContext(sws_w, sws_h, (AVPixelFormat)sws_src_fmt,
+                                             sws_w, sws_h, AV_PIX_FMT_GRAY8,
+                                             SWS_BILINEAR, nullptr, nullptr, nullptr);
+                    }
+                    if (sws) {
+                        cv::Mat gray_full(sws_h, sws_w, CV_8UC1);
+                        uint8_t* dst_data[4] = { gray_full.data, nullptr, nullptr, nullptr };
+                        int dst_linesize[4] = { static_cast<int>(gray_full.step), 0, 0, 0 };
+                        sws_scale(sws, frm->data, frm->linesize, 0, sws_h, dst_data, dst_linesize);
+
+                        // Downscale to small size for fast motion detection
+                        if (target_h == 0) {
+                            target_h = std::max(24, (int)std::lround((double)target_w * sws_h / sws_w));
+                        }
+                        cv::resize(gray_full, gray_small, cv::Size(target_w, target_h), 0, 0, cv::INTER_AREA);
+                        cv::GaussianBlur(gray_small, gray_small, cv::Size(5,5), 0);
+
+                        if (!prev_gray_small.empty()) {
+                            cv::absdiff(gray_small, prev_gray_small, diff);
+                            cv::threshold(diff, thresh, 20, 255, cv::THRESH_BINARY);
+                            // Optional: morphological open to reduce noise
+                            // cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), 1);
+                            double changed = (double)cv::countNonZero(thresh) / (thresh.total()) * 100.0;
+                            if (changed >= args.motion_thresh) {
+                                motion_active = true;
+                                last_motion = clock::now();
+                            }
+                        }
+                        gray_small.copyTo(prev_gray_small);
+                    }
+                    av_frame_unref(frm);
+                }
+                if (frm) av_frame_free(&frm);
+            }
+        }

-        // Rescale timestamps to output timebase
-        av_packet_rescale_ts(&pkt, in_st->time_base, out_st->time_base);
-        pkt.stream_index = out_st->index;
-
-        // Write packet (interleaved for proper RTP timestamping)
-        ret = av_interleaved_write_frame(out_ctx, &pkt);
-        if (ret < 0) {
-            log_ff("av_interleaved_write_frame failed", ret);
-            av_packet_unref(&pkt);
-            break;
-        }
+        // Each input video packet is treated as one frame tick for the timeline.
+        // Advance timeline regardless of whether we send or drop.
+        int64_t this_dts = timeline;
+        timeline += frame_duration;
+        frame_index++;
+
+        // Update motion active state with cooldown
+        if (args.motion_enable) {
+            if (motion_active) {
+                auto ms_since = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last_motion).count();
+                if (ms_since > args.motion_cooldown_ms) {
+                    motion_active = false;
+                }
+            }
+        }
+
+        bool send_this = true;
+        if (args.motion_enable) {
+            if (!motion_active) {
+                // Idle: send sparse keepalive if configured
+                send_this = (args.idle_send_fps > 0) && (frame_index % frames_per_idle == 0);
+            }
+        }
+
+        if (send_this) {
+            // Set clean, monotonic timestamps on the packet we send
+            av_packet_rescale_ts(&pkt, in_st->time_base, out_st->time_base);
+            pkt.stream_index = out_st->index;
+            pkt.dts = this_dts;
+            // Keep PTS >= DTS
+            pkt.pts = (pkt.pts == AV_NOPTS_VALUE) ? this_dts : std::max(pkt.pts, this_dts);
+            pkt.duration = frame_duration;
+
+            ret = av_interleaved_write_frame(out_ctx, &pkt);
+            if (ret < 0) {
+                log_ff("av_interleaved_write_frame failed", ret);
+                av_packet_unref(&pkt);
+                break;
+            }
+        }
 
         av_packet_unref(&pkt);
     }

     // Cleanup
     av_write_trailer(out_ctx);
     avformat_close_input(&in_ctx);
     avformat_free_context(out_ctx);
     avformat_network_deinit();
+    if (dec_ctx) avcodec_free_context(&dec_ctx);
+    if (sws) sws_freeContext(sws);

     std::cerr << "Stopped streaming.\n";
     return 0;
}
