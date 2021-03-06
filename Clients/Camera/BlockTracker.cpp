/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:58:49
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-22 21:30:28
 */
#include "BlockTracker.h"

#include <vector>
#include <math.h>
#include <iostream>
#include <jsoncpp/json/json.h>

void drawRect(cv::Mat &frame, cv::RotatedRect &rect)
{
    cv::Point2f vertex[4];
	rect.points(vertex);
    for (int i = 0; i < 4; i++) {
        cv::line(frame, vertex[i], vertex[(i + 1) % 4], cv::Scalar(255, 100, 200), 3);
    }
}

float pointDist(cv::Point2f &p1, cv::Point2f &p2)
{
    float dist = powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2);
    dist = sqrtf(dist);
    return dist;
}

BlockTracker::BlockTracker(const Json::Value &cfg_json):
    speed(0), next_block_id(0),
    last_update_time(std::chrono::system_clock::now())
{
    x = cfg_json["x"].asDouble();
    y = cfg_json["y"].asDouble();

    Json::Value min = cfg_json["rgb_min"];
    Json::Value max = cfg_json["rgb_max"];
    rgb_min = cv::Scalar(min["R"].asDouble(),
                         min["G"].asDouble(),
                         min["B"].asDouble());
    rgb_max = cv::Scalar(max["R"].asDouble(),
                         max["G"].asDouble(),
                         max["B"].asDouble());

    position_area_max = cfg_json["position_area_max"].asDouble();
    position_area_min = cfg_json["position_area_min"].asDouble();

    area_min = cfg_json["area_min"].asDouble();
    area_max = cfg_json["area_max"].asDouble();

    threshold_x = cfg_json["threshold_x"].asDouble();
    threshold_y = cfg_json["threshold_y"].asDouble();

    actual_width = cfg_json["actual_width"].asDouble();
    actual_height = cfg_json["actual_height"].asDouble();
}

uint32_t BlockTracker::blockNum()
{
    return blocks.size();
}

std::string BlockTracker::toString()
{
    Json::Value msg_json;
    msg_json["source"] = "camera";
    msg_json["message_type"] = "normal";
    msg_json["speed"] = speed;
    msg_json["timestamp"] = std::to_string(
        (uint64_t)last_update_time.time_since_epoch().count());
    
    Json::Value blk_arr;
    for (const Block &block : blocks) {
        Json::Value blk;

        blk["id"] = block.block_id;
        blk["x"] = block.x;
        blk["y"] = block.y;
        blk["angle"] = block.angle;

        blk_arr.append(blk);
    }
    msg_json["blocks"] = blk_arr;
    return Json::FastWriter().write(msg_json);
    // return msg_json.toStyledString();
}

std::string BlockTracker::toHeartbeatString()
{
    Json::Value msg_json;
    msg_json["source"] = "camera";
    msg_json["message_type"] = "heartbeat";
    msg_json["x"] = x;
    msg_json["y"] = y;
    msg_json["w"] = actual_width;
    msg_json["h"] = actual_height;
    return Json::FastWriter().write(msg_json);
    // return msg_json.toStyledString();
}

bool BlockTracker::calibrate(cv::Mat &frame)
{
    frame_width = frame.cols;
    frame_height = frame.rows;
    detect_start_x = frame_width * 0.07;
    detect_end_x = frame_width - detect_start_x;

    float scale;
    
    cv::Mat cvtProcessed, colorSelected;
    cv::cvtColor(frame, cvtProcessed, cv::COLOR_BGR2Lab);
    cv::inRange(cvtProcessed, rgb_min, rgb_max, colorSelected);

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(colorSelected, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    uint32_t detected_blocks_num = 0;
    std::vector<cv::Point2f> blocks;

    for (std::vector< std::vector<cv::Point> >::iterator contour = contours.begin();
         contour != contours.end() && detected_blocks_num < 4; ++contour) {
        
        cv::RotatedRect rotatedRect = cv::minAreaRect(*contour);
        float area = rotatedRect.size.area();
        
        if (area < position_area_max && area > position_area_min) {
            detected_blocks_num += 1;
            blocks.insert(blocks.end(), contour->begin(), contour->end());
        }
    }
    if (detected_blocks_num < 4) return false;

    cv::RotatedRect border = cv::minAreaRect(blocks);
    
    cv::Point2f vertices[4];
    border.points(vertices);
    cv::Point2f bottom_left = vertices[0];
    cv::Point2f top_left = vertices[1];
    cv::Point2f top_right = vertices[2];
    cv::Point2f bottom_right = vertices[3];

    float dist_lr = pointDist(top_left, top_right);
    float dist_bt = pointDist(bottom_left, top_left);

    if (dist_lr <= dist_bt) return false;

    scale = actual_width / dist_lr;
    
    sin_val = (top_right.y - top_left.y) / dist_lr * scale;
    cos_val = (top_right.x - top_left.x) / dist_lr * scale;
    angle = asinf(sin_val) / M_PI * 180;

    printf("[Cal]\tscale: %.4fx,\tangle: %.2f??\n", scale, angle);

    offset_x = top_left.x;
    offset_y = top_left.y;

    drawRect(frame, border);
    cv::imwrite("bound.png", frame);
    
    return true;
}

cv::Point2f BlockTracker::toReal(const cv::Point2f &camera_point)
{
    float x = camera_point.x;
    float y = camera_point.y;

    float real_x = cos_val * (x - offset_x) + sin_val * (y - offset_y);
    float real_y = - sin_val * (x - offset_x) + cos_val * (y - offset_y);

    return cv::Point2f(real_x, real_y);
}

float BlockTracker::updateBlock(const cv::RotatedRect &rect, int64_t interval_ms)
{
    cv::Point2f real_point = toReal(rect.center);
    float expected_dist_x = speed * interval_ms;
    float last_expected_x = real_point.x - expected_dist_x;
    float last_expected_y = real_point.y;

    for (std::list<Block>::iterator block = blocks.begin();
        block != blocks.end(); ++block) {

        float diff_x = block->x - last_expected_x;
        float diff_y = block->y - last_expected_y;

        if (diff_x < threshold_x && diff_x > (-threshold_x)
            && diff_y < threshold_y && diff_y > (-threshold_y)) {

            float old_center_x = block->x;
            float new_center_x = (old_center_x + expected_dist_x + 3 * real_point.x) / 4;
            block->x = new_center_x;
            block->y = (block->y + 3 * real_point.y) / 4;
            block->angle = (block->angle + 3 * rect.angle) / 4;
            block->updated = true;
            return new_center_x - old_center_x;
        }
    }

    blocks.push_back(Block(next_block_id++, real_point.x, real_point.y, rect.angle));
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
        
        if (area < area_max && area > area_min
            && rotatedRect.center.x < detect_end_x
            && rotatedRect.center.x > detect_start_x) {
            // drawRect(frame, rotatedRect);
            float dist_x = updateBlock(rotatedRect, interval_ms);
            total_dist_x += dist_x;
            detected_blocks_num += 1;
        }
    }

    for (std::list<Block>::iterator block = blocks.begin();
        block != blocks.end(); ++block) {
        
        if (block->updated) {
            block->updated = false;
        } else {
            block = blocks.erase(block);
            --block;
        }
    }
    
    if (detected_blocks_num > 0)
        speed = total_dist_x / (detected_blocks_num * interval_ms);
    last_update_time = capture_time;
    
    for (std::list<Block>::iterator block = blocks.begin();
        block != blocks.end(); ++block) {
        
        printf("[Block]\tid: %d,\tx: %.2f mm,\ty: %.2f mm,\tspeed: %.4f m/s,\ttime: %ld ms\n",
            block->block_id, block->x, block->y, speed, interval_ms);
    }
}