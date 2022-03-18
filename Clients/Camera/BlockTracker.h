/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:32:02
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-18 20:05:36
 */
#include <list>
#include <vector>
#include <chrono>
#include <stdint.h>
#include "opencv2/opencv.hpp"

void drawRect(cv::Mat &frame, cv::RotatedRect &rect);

struct Block
{
    float x, y;
    uint32_t block_id;
    
    Block(float x, float y, uint32_t block_id):
        x(x), y(y), block_id(block_id) {}
};

class BlockTracker
{
private:
    cv::Scalar rgb_min = cv::Scalar(0, 151, 100);
    cv::Scalar rgb_max = cv::Scalar(255, 255, 255);

    float area_min = 5000;
    float area_max = 100000;

    float threshold_x = 30;
    float threshold_y = 20;
    
    uint32_t frame_width;
    uint32_t frame_height;

    float speed;
    uint32_t next_block_id;
    std::chrono::system_clock::time_point last_update_time;
    std::list<Block> blocks;

    cv::Point2f toReal(cv::Point2f &camera_point);
    float updatePoint(cv::Point2f &camera_point, int64_t interval_ms);

public:
    BlockTracker(): speed(0), next_block_id(0),
        last_update_time(std::chrono::system_clock::now()) {}
    ~BlockTracker() {}

    void update(cv::Mat &frame,
                std::chrono::system_clock::time_point capture_time);
};