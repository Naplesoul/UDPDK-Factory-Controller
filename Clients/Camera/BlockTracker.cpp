/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:58:49
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-18 20:05:40
 */
#include "BlockTracker.h"

#include <vector>
#include <iostream>

void drawRect(cv::Mat &frame, cv::RotatedRect &rect)
{
    cv::Point2f vertex[4];
	rect.points(vertex);
    for (int i = 0; i < 4; i++) {
        cv::line(frame, vertex[i], vertex[(i + 1) % 4], cv::Scalar(255, 100, 200), 3);
    }
}

cv::Point2f BlockTracker::toReal(cv::Point2f &camera_point)
{
    return camera_point;
}

float BlockTracker::updatePoint(cv::Point2f &camera_point, int64_t interval_ms)
{
    cv::Point2f real_point = toReal(camera_point);
    float expected_dist_x = speed * interval_ms;
    float last_expected_x = real_point.x - expected_dist_x;
    float last_expected_y = real_point.y;

    for (std::list<Block>::iterator block = blocks.begin();
        block != blocks.end(); ++ block) {

        float diff_x = block->x - last_expected_x;
        float diff_y = block->y - last_expected_y;

        if (diff_x < threshold_x && diff_x > (-threshold_x)
            && diff_y < threshold_y && diff_y > (-threshold_y)) {

            float old_center_x = block->x;
            float new_center_x = (old_center_x + expected_dist_x + 3 * real_point.x) / 4;
            block->x = new_center_x;
            block->y = (block->y + 3 * real_point.y) / 4;
            return new_center_x - old_center_x;
        }
    }

    blocks.push_back(Block(real_point.x, real_point.y, next_block_id++));
    return expected_dist_x;
}

void BlockTracker::update(cv::Mat &frame,
                          std::chrono::system_clock::time_point capture_time)
{
    int64_t interval_ms = std::chrono::duration_cast<std::chrono::milliseconds>
                            (capture_time - last_update_time).count();

    cv::Mat cvtProcessed, colorSelected;
    cv::cvtColor(frame, cvtProcessed, cv::COLOR_BGR2Lab);
    cv::inRange(cvtProcessed, rgb_min, rgb_max, colorSelected);

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(colorSelected, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    float total_dist_x = 0;
    uint32_t detected_blocks_num = 0;

    for (std::vector< std::vector<cv::Point> >::iterator contour = contours.begin();
         contour != contours.end(); ++contour) {
        
        cv::RotatedRect rotatedRect = cv::minAreaRect(*contour);
        float area = rotatedRect.size.area();
        
        if (area < area_max && area > area_min) {
            // drawRect(frame, rotatedRect);
            float dist_x = updatePoint(rotatedRect.center, interval_ms);
            total_dist_x += dist_x;
            detected_blocks_num += 1;
        }
    }
    
    if (detected_blocks_num > 0)
        speed = total_dist_x / (detected_blocks_num * interval_ms);
    last_update_time = capture_time;
    
    for (std::list<Block>::iterator block = blocks.begin();
        block != blocks.end(); ++ block) {
        
        std::cout << "Block[" << block->block_id << "] x: " << block->x << ", y: "
            << block->y << ", speed: " << speed << ", time_interval: " << interval_ms << std::endl;
    }
}