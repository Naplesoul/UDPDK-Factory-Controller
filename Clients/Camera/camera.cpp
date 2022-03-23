/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:32:09
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-22 21:31:19
 */
#include <chrono>
#include <unistd.h>
#include "opencv2/opencv.hpp"

#include "BlockTracker.h"

int main()
{
    cv::VideoCapture capture(0);
    cv::Mat frame;
    BlockTracker tracker;
    sleep(1);

    do {
        capture.read(frame);
    } while (!tracker.calibrate(frame));

    sleep(10);
    while (capture.read(frame)) {
        tracker.update(frame, std::chrono::system_clock::now());
        usleep(50000);
    }
}