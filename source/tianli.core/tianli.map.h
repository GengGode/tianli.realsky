#pragma once

#include "core.map/item.set.h"
#include "core.map/map.resource.h"

class Map
{
public:
    Map()
    {
        // set
        set = std::make_shared<MapSet>();
        // set->load("./item/item_all.json", cv::Point(232, 216));
        // resource
        resource = std::make_shared<MapResource>();
        resource->load("./map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    }
    ~Map() = default;

public:
    cv::Mat view(const MapSprite &sprite)
    {
        // cv::Mat viewer;
        //  auto width = sprite.size.width / sprite.scale;
        //  auto height = sprite.size.height / sprite.scale;
        //  auto rect = cv::Rect2d(sprite.pos.x - width * 0.5, sprite.pos.y - height * 0.5, width, height);
        //  auto rs = set->find(rect);
        //  auto mat = resource->view(rect).clone();
        auto viewer = border_mask(sprite.size);
        // cv::resize(mat, viewer, sprite.size);
        // cv::imshow("viewer", viewer);
        // cv::waitKey(1);
        return viewer;
    }
    void set_mask(cv::Mat mask)
    {
        this->mask = mask;
    }

    cv::Mat border_mask(cv::Size size)
    {
        return border_mask(this->mask, size.width, size.height, 30, 30, 30, 30);
    }

    cv::Mat border_mask(cv::Mat src, int w, int h, int clip_top, int clip_right, int clip_bottom, int clip_left)
    {
        assert(src.channels() == 4);
        cv::Mat mask = cv::Mat::zeros(h, w, CV_8UC4);
        //  target, source
        std::vector<std::pair<cv::Rect, cv::Rect>> rects = {
            // left_top
            {cv::Rect(0, 0, clip_left, clip_top), cv::Rect(0, 0, clip_left, clip_top)},
            // left_bottom
            {cv::Rect(0, h - clip_bottom, clip_left, clip_bottom), cv::Rect(0, src.rows - clip_bottom, clip_left, clip_bottom)},
            // right_top
            {cv::Rect(w - clip_right, 0, clip_right, clip_top), cv::Rect(src.cols - clip_right, 0, clip_right, clip_top)},
            // right_bottom
            {cv::Rect(w - clip_right, h - clip_bottom, clip_right, clip_bottom), cv::Rect(src.cols - clip_right, src.rows - clip_bottom, clip_right, clip_bottom)},
            // top
            {cv::Rect(clip_left, 0, w - clip_left - clip_right, clip_top), cv::Rect(clip_left, 0, src.cols - clip_left - clip_right, clip_top)},
            // bottom
            {cv::Rect(clip_left, h - clip_bottom, w - clip_left - clip_right, clip_bottom), cv::Rect(clip_left, src.rows - clip_bottom, src.cols - clip_left - clip_right, clip_bottom)},
            // left
            {cv::Rect(0, clip_top, clip_left, h - clip_top - clip_bottom), cv::Rect(0, clip_top, clip_left, src.rows - clip_top - clip_bottom)},
            // right
            {cv::Rect(w - clip_right, clip_top, clip_right, h - clip_top - clip_bottom), cv::Rect(src.cols - clip_right, clip_top, clip_right, src.rows - clip_top - clip_bottom)},
            // center
            {cv::Rect(clip_left, clip_top, w - clip_left - clip_right, h - clip_top - clip_bottom), cv::Rect(clip_left, clip_top, src.cols - clip_left - clip_right, src.rows - clip_top - clip_bottom)},
        };
        for (auto &[target, source] : rects)
        {
            if (target.width <= 0 || target.height <= 0)
                continue;
            auto target_mat = mask(target);
            auto source_mat = src(source);
            if (source_mat.size() != target_mat.size())
            {
                cv::resize(source_mat, source_mat, target_mat.size());
            }
            source_mat.copyTo(target_mat);
        }
        return mask;
    }

public:
    cv::Mat mask;
    std::shared_ptr<MapSet> set;
    std::shared_ptr<MapResource> resource;
};