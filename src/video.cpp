/*
 * Push /dev/video0 to RTSP server (no vendor SDK)
 * - Captures via FFmpeg v4l2 input
 * - Copies encoded packets (H.264 or MJPEG) directly to RTSP output
 * - No re-encoding by default
 *
 * Build:
 *   g++ rtsp_push.cpp -o rtsp_push -lavformat -lavdevice -lavcodec -lavutil -lpthread
 *
 * Example:
 *   ./rtsp_push --url rtsp://server:8554/stream --input /dev/video0 --format h264 --size 1280x720 --fps 30 --transport tcp
 */

#include <thread>

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavdevice/avdevice.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libswscale/swscale.h>
}

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <utility>
#include <chrono>
#include <cmath>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

static volatile std::sig_atomic_t g_stop = 0;

static void handle_sigint(int)
{
	g_stop = 1;
}

static void log_ff(const std::string& msg, int err = 0)
{
	if (err < 0)
	{
		char buf[256];
		av_strerror(err, buf, sizeof(buf));
		std::cerr << msg << ": " << buf << "\n";
	} else
	{
		std::cerr << msg << "\n";
	}
}


struct Args
{
	std::string input = "/dev/video0";
	std::string url;                   // rtsp://...
	std::string format       = "";           // "h264" or "mjpeg" (optional hint)
	std::string size         = "";             // e.g. "1280x720"
	std::string transport    = "tcp";     // "tcp" or "udp"
	int         fps          = 0;                       // optional
	int         max_delay_ms = 500;            // network max_delay

	int    motion_enable      = 1;
	int    motion_history     = 500;
	double motion_thresh      = 10.0;
	int    motion_cooldown_ms = 5000;
	int    motion_scale_w     = 160;
	int    idle_send_fps      = 0;
};


static Args parse_args(int argc, char** argv)
{
	Args a;
	for (int i = 1; i < argc; ++i)
	{
		std::string k    = argv[i];
		auto        next = [&](std::string def = "") -> std::string {
			if (i + 1 < argc)
				return std::string(argv[++i]);
			return def;
		};
		if (k == "--input")
			a.input = next();
		else if (k == "--url")
			a.url = next();
		else if (k == "--format")
			a.format = next();       // "h264" or "mjpeg"
		else if (k == "--size")
			a.size = next();           // "WIDTHxHEIGHT"
		else if (k == "--fps")
			a.fps = std::atoi(next().c_str());
		else if (k == "--transport")
			a.transport = next(); // "tcp" or "udp"
		else if (k == "--max_delay_ms")
			a.max_delay_ms = std::atoi(next().c_str());
		else if (k == "--motion_enable")
			a.motion_enable = std::atoi(next().c_str());
		else if (k == "--motion_thresh")
			a.motion_thresh = std::atof(next().c_str());
		else if (k == "--motion_cooldown_ms")
			a.motion_cooldown_ms = std::atoi(next().c_str());
		else if (k == "--motion_scale_w")
			a.motion_scale_w = std::atoi(next().c_str());
		else if (k == "--idle_send_fps")
			a.idle_send_fps = std::atoi(next().c_str());
		else if (k == "--help" || k == "-h")
		{
			std::cout <<
				"Usage: rtsp_push --url rtsp://host:port/path [--input /dev/video0]\n"
				"                 [--format h264|mjpeg] [--size 1280x720] [--fps 30]\n"
				"                 [--transport tcp|udp] [--max_delay_ms 500]\n"
				"                 [--transport tcp|udp] [--max_delay_ms 500]\n"
				"Motion gating:\n"
				"  --motion_enable 1            enable motion gating (0/1)\n"
				"  --motion_thresh 10.0         % pixels changed to trigger motion\n"
				"  --motion_cooldown_ms 5000    keep streaming this long after motion\n"
				"  --motion_scale_w 160         downscale width for motion processing\n"
				"  --idle_send_fps 0            send FPS while idle (0 = drop all)\n";

			std::exit(0);
		}
	}
	return a;
}

int main3(int argc, char** argv)
{
	std::signal(SIGINT, handle_sigint);
	std::signal(SIGTERM, handle_sigint);

	Args args = parse_args(argc, argv);
	if (args.url.empty())
	{
		std::cerr << "Error: --url rtsp://server:port/stream is required\n";
		return 1;
	}

	avdevice_register_all();
	avformat_network_init();

	int ret = 0;

	// Open V4L2 input
	const AVInputFormat* ifmt = av_find_input_format("video4linux2");
	if (!ifmt)
	{
		log_ff("Failed to find input format video4linux2");
		return 1;
	}

	AVDictionary* in_opts = nullptr;
	// Optional: select input format from camera if known
	// Common options:
	//   input_format=h264 (for H.264)
	//   input_format=mjpeg (for MJPEG)
	if (!args.format.empty())
	{
		av_dict_set(&in_opts, "input_format", args.format.c_str(), 0);
	}
	if (!args.size.empty())
	{
		av_dict_set(&in_opts, "video_size", args.size.c_str(), 0);
	}
	if (args.fps > 0)
	{
		av_dict_set(&in_opts, "framerate", std::to_string(args.fps).c_str(), 0);
	}
	// Some drivers need 'seekable' = 0 to avoid blocking
	av_dict_set(&in_opts, "seekable", "0", 0);

	// Prefer wallclock-based timestamps from the demuxer (helps some drivers)
	av_dict_set(&in_opts, "use_wallclock_as_timestamps", "1", 0);

	AVFormatContext* in_ctx = nullptr;
	if ((ret = avformat_open_input(&in_ctx, args.input.c_str(), const_cast<AVInputFormat*>(ifmt), &in_opts)) < 0)
	{
		log_ff("avformat_open_input failed", ret);
		av_dict_free(&in_opts);
		return 1;
	}
	av_dict_free(&in_opts);

	if ((ret = avformat_find_stream_info(in_ctx, nullptr)) < 0)
	{
		log_ff("avformat_find_stream_info failed", ret);
		avformat_close_input(&in_ctx);
		return 1;
	}

	// Find the best video stream
	int video_stream_index = av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_index < 0)
	{
		log_ff("No video stream found");
		avformat_close_input(&in_ctx);
		return 1;
	}
	AVStream* in_st = in_ctx->streams[video_stream_index];

	// Log input codec
	const AVCodecParameters* in_par     = in_st->codecpar;
	std::string              codec_name = in_par->codec_id == AV_CODEC_ID_H264 ?
											  "H264" :
											  in_par->codec_id == AV_CODEC_ID_MJPEG ?
											  "MJPEG" :
											  avcodec_get_name(in_par->codec_id);
	std::cerr << "Input stream codec: " << codec_name
		<< " " << in_par->width << "x" << in_par->height << "\n";

	const AVCodec*  dec     = avcodec_find_decoder(in_par->codec_id);
	AVCodecContext* dec_ctx = nullptr;
	if (!dec)
	{
		log_ff("Decoder not found for motion processing");
	} else
	{
		dec_ctx = avcodec_alloc_context3(dec);
		if (!dec_ctx)
		{
			log_ff("avcodec_alloc_context3 failed for decoder");
		} else
		{
			if (avcodec_parameters_to_context(dec_ctx, in_par) < 0)
			{
				log_ff("avcodec_parameters_to_context failed for decoder");
				avcodec_free_context(&dec_ctx);
				dec_ctx = nullptr;
			} else if (avcodec_open2(dec_ctx, dec, nullptr) < 0)
			{
				log_ff("avcodec_open2 failed for decoder");
				avcodec_free_context(&dec_ctx);
				dec_ctx = nullptr;
			}
		}
	}

	// Sws context to convert decoded frames to GRAY8
	SwsContext* sws   = nullptr;
	int         sws_w = 0, sws_h = 0, sws_src_fmt = AV_PIX_FMT_NONE;

	// Prepare RTSP output
	AVFormatContext* out_ctx = nullptr;
	if ((ret = avformat_alloc_output_context2(&out_ctx, nullptr, "rtsp", args.url.c_str())) < 0 || !out_ctx)
	{
		log_ff("avformat_alloc_output_context2 (rtsp) failed", ret);
		avformat_close_input(&in_ctx);
		return 1;
	}

	// Set RTSP options
	AVDictionary* out_opts = nullptr;
	// Transport: tcp or udp
	av_dict_set(&out_opts, "rtsp_transport", args.transport.c_str(), 0);
	// Lower latency preferences
	av_dict_set(&out_opts, "muxdelay", "0.0", 0);
	av_dict_set(&out_opts, "muxpreload", "0.0", 0);
	// Max RTP delay (microseconds). Convert ms to us
	av_dict_set(&out_opts, "max_delay", std::to_string(args.max_delay_ms * 1000).c_str(), 0);
	// Some servers want 'stimeout' in us for socket timeouts (e.g., 5s)
	av_dict_set(&out_opts, "stimeout", "5000000", 0);

	// Create output stream and copy codec parameters
	AVStream* out_st = avformat_new_stream(out_ctx, nullptr);
	if (!out_st)
	{
		log_ff("avformat_new_stream failed");
		av_dict_free(&out_opts);
		avformat_close_input(&in_ctx);
		avformat_free_context(out_ctx);
		return 1;
	}

	if ((ret = avcodec_parameters_copy(out_st->codecpar, in_par)) < 0)
	{
		log_ff("avcodec_parameters_copy failed", ret);
		av_dict_free(&out_opts);
		avformat_close_input(&in_ctx);
		avformat_free_context(out_ctx);
		return 1;
	}
	// Clear codec_tag to prevent muxer confusion
	out_st->codecpar->codec_tag = 0;

	// Timebase: use input stream timebase if present, else default
	out_st->time_base = in_st->time_base.num && in_st->time_base.den ? in_st->time_base : AVRational{1, 90000};

	// Print destination
	av_dump_format(out_ctx, 0, args.url.c_str(), 1);

	out_st->time_base = AVRational{1, 90000};
	AVRational fps_q  = in_st->avg_frame_rate.num && in_st->avg_frame_rate.den ?
							in_st->avg_frame_rate :
							(in_st->r_frame_rate.num && in_st->r_frame_rate.den ?
								 in_st->r_frame_rate :
								 AVRational{args.fps > 0 ? args.fps : 30, 1});
	double fps = av_q2d(fps_q);
	if (fps < 1.0 || !std::isfinite(fps))
	{
		fps   = (args.fps > 0) ? args.fps : 30.0;
		fps_q = AVRational{static_cast<int>(fps + 0.5), 1};
	}
	auto frame_duration = static_cast<int64_t>(std::llround(90000.0 / fps));
	if (frame_duration < 1)
		frame_duration = 1;

	int64_t next_dts = 0;
	int64_t next_pts = 0;

	// Open output (RTSP doesn't use avio_open; ffmpeg handles sockets internally)
	if ((ret = avformat_write_header(out_ctx, &out_opts)) < 0)
	{
		log_ff("avformat_write_header (RTSP ANNOUNCE/SETUP) failed", ret);
		av_dict_free(&out_opts);
		avformat_close_input(&in_ctx);
		avformat_free_context(out_ctx);
		return 1;
	}
	av_dict_free(&out_opts);

	std::cerr << "Streaming to " << args.url << " (Press Ctrl+C to stop)\n";

	// Main forwarding loop
	AVPacket pkt;
	av_init_packet(&pkt);

	using clock          = std::chrono::steady_clock;
	auto last_motion     = clock::time_point::min();
	bool motion_active   = false;
	int  frames_per_idle = (args.idle_send_fps > 0) ?
							   std::max(1, static_cast<int>(std::lround(fps / args.idle_send_fps))) :
							   INT32_MAX;

	cv::Mat  prev_gray_small, gray_small, diff, thresh;
	int      target_w    = std::max(32, args.motion_scale_w);
	int      target_h    = 0; // computed after first frame
	uint64_t frame_index = 0;

	while (!g_stop)
	{
		ret = av_read_frame(in_ctx, &pkt);
		if (ret == AVERROR(EAGAIN))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		} else if (ret < 0)
		{
			// End or error. Try a short wait and continue, or break
			log_ff("av_read_frame returned", ret);
			break;
		}

		if (pkt.stream_index != video_stream_index)
		{
			av_packet_unref(&pkt);
			continue;
		}

		if (dec_ctx && args.motion_enable)
		{
			AVPacket pkt_copy = pkt; // avcodec_send_packet takes ownership only on success; safe to copy
			if (avcodec_send_packet(dec_ctx, &pkt_copy) >= 0)
			{
				AVFrame* frm = av_frame_alloc();
				while (frm && avcodec_receive_frame(dec_ctx, frm) == 0)
				{
					// Prepare sws on first frame or if format changed
					if (!sws || sws_w != frm->width || sws_h != frm->height || sws_src_fmt != frm->format)
					{
						if (sws)
							sws_freeContext(sws);
						sws_w       = frm->width;
						sws_h       = frm->height;
						sws_src_fmt = frm->format;
						sws         = sws_getContext(sws_w,
													 sws_h,
													 (AVPixelFormat) sws_src_fmt,
													 sws_w,
													 sws_h,
													 AV_PIX_FMT_GRAY8,
													 SWS_BILINEAR,
													 nullptr,
													 nullptr,
													 nullptr);
					}
					if (sws)
					{
						cv::Mat  gray_full(sws_h, sws_w, CV_8UC1);
						uint8_t* dst_data[4]     = {gray_full.data, nullptr, nullptr, nullptr};
						int      dst_linesize[4] = {static_cast<int>(gray_full.step), 0, 0, 0};
						sws_scale(sws, frm->data, frm->linesize, 0, sws_h, dst_data, dst_linesize);

						// Downscale to small size for fast motion detection
						if (target_h == 0)
						{
							target_h = std::max(24, (int) std::lround((double) target_w * sws_h / sws_w));
						}
						cv::resize(gray_full, gray_small, cv::Size(target_w, target_h), 0, 0, cv::INTER_AREA);
						cv::GaussianBlur(gray_small, gray_small, cv::Size(5, 5), 0);

						if (!prev_gray_small.empty())
						{
							cv::absdiff(gray_small, prev_gray_small, diff);
							cv::threshold(diff, thresh, 20, 255, cv::THRESH_BINARY);
							// Optional: morphological open to reduce noise
							// cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), 1);
							double changed = (double) cv::countNonZero(thresh) / (thresh.total()) * 100.0;
							if (changed >= args.motion_thresh)
							{
								motion_active = true;
								last_motion   = clock::now();
							}
						}
						gray_small.copyTo(prev_gray_small);
					}
					av_frame_unref(frm);
				}
				if (frm)
					av_frame_free(&frm);
			}
		}

		if (args.motion_enable)
		{
			if (motion_active)
			{
				auto ms_since = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last_motion).
					count();
				if (ms_since > args.motion_cooldown_ms)
				{
					motion_active = false;
				}
			}
		}

		frame_index++;

		bool send_this = true;
		if (args.motion_enable)
		{
			if (!motion_active)
			{
				// Idle: send sparse keepalive if configured
				send_this = (args.idle_send_fps > 0) && (frame_index % frames_per_idle == 0);
			}
		}

		if (send_this)
		{
			// Discard input timing; synthesize clean RTP timeline
			// Optionally, still rescale for completeness before overwrite
			av_packet_rescale_ts(&pkt, in_st->time_base, out_st->time_base);
			pkt.stream_index = out_st->index;

			pkt.dts      = next_dts;
			pkt.pts      = next_pts;
			pkt.duration = frame_duration;

			next_dts += frame_duration;
			// Keep PTS >= DTS; for IP-only streams we advance equally
			next_pts = std::max(next_dts, next_pts + frame_duration);


			// Write packet (interleaved for proper RTP timestamping)
			ret = av_interleaved_write_frame(out_ctx, &pkt);
			if (ret < 0)
			{
				log_ff("av_interleaved_write_frame failed", ret);
				av_packet_unref(&pkt);
				break;
			}
		}

		av_packet_unref(&pkt);
	}

	// Cleanup
	av_write_trailer(out_ctx);
	avformat_close_input(&in_ctx);
	avformat_free_context(out_ctx);
	avformat_network_deinit();
	if (dec_ctx)
		avcodec_free_context(&dec_ctx);
	if (sws)
		sws_freeContext(sws);

	std::cerr << "Stopped streaming.\n";
	return 0;
}
