/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:32:09
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-22 21:31:19
 */
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "opencv2/opencv.hpp"

#include "BlockTracker.h"

int main(int argc,char *argv[])
{
    int config_fd = open(argv[1], O_RDONLY|O_NDELAY);
    if (config_fd < 0) {
        printf("Fail to open config json file: %s", argv[1]);
        return -1;
    }

    struct stat *config_stat = (struct stat *)malloc(sizeof(struct stat));
    stat(argv[1], config_stat);
    char *config_buf = (char *)malloc(config_stat->st_size);
    int read_count = read(config_fd, config_buf, config_stat->st_size);
    if (read_count != config_stat->st_size) {
        printf("Fail to read from config json file: %s", argv[1]);
        return -1;
    }
    
    close(config_fd);
    free(config_stat);

    cv::VideoCapture capture(0);
    cv::Mat frame;
    BlockTracker tracker(config_buf);
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