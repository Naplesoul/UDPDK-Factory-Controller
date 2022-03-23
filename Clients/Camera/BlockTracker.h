/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:32:02
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-22 21:30:42
 */
#include <list>
#include <vector>
#include <chrono>
#include <stdint.h>
#include "opencv2/opencv.hpp"

void drawRect(cv::Mat &frame, cv::RotatedRect &rect);
float pointDist(cv::Point2f &p1, cv::Point2f &p2);

struct Block
{
    bool updated;
    uint32_t block_id;
    float x, y, angle;
    
    Block(uint32_t block_id, float x, float y, float angle):
        updated(true), block_id(block_id), x(x), y(y), angle(angle) {}
};

class BlockTracker
{
private:
    cv::Scalar rgb_min = cv::Scalar(0, 151, 100);
    cv::Scalar rgb_max = cv::Scalar(255, 255, 255);

    float position_area_min = 300;
    float position_area_max = 1000;

    float area_min = 3000;
    float area_max = 100000;

    float threshold_x = 30;
    float threshold_y = 20;

    float actual_width = 210;

    uint32_t frame_width;
    uint32_t frame_height;

    float detect_start_x;
    float detect_end_x;

    float sin_val;
    float cos_val;
    float offset_x;
    float offset_y;

    float speed;
    uint32_t next_block_id;
    std::chrono::system_clock::time_point last_update_time;
    std::list<Block> blocks;

    cv::Point2f toReal(const cv::Point2f &camera_point);
    float updateBlock(const cv::RotatedRect &rect, int64_t interval_ms);

public:
    BlockTracker(): speed(0), next_block_id(0),
        last_update_time(std::chrono::system_clock::now()) {}
    ~BlockTracker() {}

    bool calibrate(cv::Mat &frame);
    void update(cv::Mat &frame,
                std::chrono::system_clock::time_point capture_time);
};