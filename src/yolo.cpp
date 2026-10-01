#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
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

int main(int argc, char** argv)
{
    std::cout << "Milk-V Duo S YOLOv8 Hardware Video Streamer\n";
    return 0;
}
