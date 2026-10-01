#include <iostream>
#include <thread>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <blt/parse/argparse_v2.h>
#include <event2/http.h>

int main2(const int argc, const char** argv)
{
	blt::argparse::argument_parser_t parser{};
	parser.add_flag("--interval").set_default(1.f).as_type<float>().set_dest("interval");
	parser.add_flag("--frame_width").set_default(1280).as_type<int>().set_dest("frame_width");
	parser.add_flag("--frame_height").set_default(720).as_type<int>().set_dest("frame_height");

	auto args = parser.parse(argc, argv);

	int cameraIndex = 0;
	cv::VideoCapture cap(cameraIndex, cv::CAP_ANY);
	if (!cap.isOpened()) {
		std::cerr << "Failed to open camera " << cameraIndex << std::endl;
		return 1;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, args.get<int>("frame_width"));
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, args.get<int>("frame_height"));
	cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));

	std::vector<int> jpeg_params = {cv::IMWRITE_JPEG_QUALITY, 95};
	cv::Mat frame;

	using clock = std::chrono::steady_clock;
	const auto interval = std::chrono::milliseconds(static_cast<int>(args.get<float>("interval") * 1000));
	auto next_tick = clock::now();

	while (true)
	{
		if (!cap.read(frame) || frame.empty()) {
			std::cerr << "Failed to read frame\n";
			// Short pause to avoid busy error looping
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		auto now = clock::now();
		if (now >= next_tick) {
			std::string fname = "file.jpg";
			if (!cv::imwrite(fname, frame, jpeg_params)) {
				std::cerr << "Failed to write image: " << fname << "\n";
			} else {
				std::cout << "Saved " << fname << "\n";
			}
			// Schedule next tick on a strict cadence
			do {
				next_tick += interval;
			} while (next_tick <= now);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	return 0;
}