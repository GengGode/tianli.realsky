#pragma once
#include <opencv2/xfeatures2d.hpp>
#include <meojson/include/json.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>

#include "RectSort.h"
#include "utils.image.h"

struct overlay_info
{
    std::string lable;
    cv::Mat image;
    cv::Point tl;
    cv::Point br;
    cv::Rect rect() { return cv::Rect(tl, image.size()); }
};

std::vector<overlay_info> from_json_overlay(std::string json_file)
{
    std::vector<overlay_info> overlays;
    std::ifstream in(json_file);
    std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto json_res = json::parse(str);
    if (json_res.has_value() == false)
    {
        return overlays;
    }
    auto json = json_res.value();
    auto type = json.type();
    auto all_object = json["plugins"].as_object();
    for (auto &&[map_name, value] : all_object)
    {
        auto map_object = value.as_object();
        if (map_object.find("overlay").has_value() == false)
            continue;
        for (auto &overlay : map_object["overlayConfig"]["overlays"].as_array())
        {
            for (auto &child : overlay["children"].as_array())
            {
                for (auto &chunk : child["chunks"].as_array())
                {
                    auto value = chunk["value"].as_string();
                    auto bounds = chunk["bounds"].as_array();
                    auto x1 = bounds[0][0].as_double();
                    auto y1 = bounds[0][1].as_double();
                    auto x2 = bounds[1][0].as_double();
                    auto y2 = bounds[1][1].as_double();
                    overlays.push_back({value + ".png", cv::Mat(), cv::Point(x1, y1), cv::Point(x2, y2)});
                }
            }
        }
    }
    for (auto &overlay : overlays)
    {
        // std::cout << overlay.lable << std::endl;
        if (std::filesystem::exists(std::filesystem::path("./overlay") / overlay.lable))
        {
            overlay.image = cv::imread((std::filesystem::path("./overlay") / overlay.lable).string(), -1);
        }
        else
        {
            std::cout << overlay.lable << "not exists" << std::endl;
        }
    }
    return overlays;
}

std::map<cv::Size, std::vector<overlay_info>::iterator> size_map(std::vector<overlay_info> &overlays)
{
    std::map<cv::Size, std::vector<overlay_info>::iterator> size_map;
    for (auto it = overlays.begin(); it != overlays.end(); it++)
    {
        size_map[it->image.size()] = it;
    }
    return size_map;
}

std::vector<cv::Size> size_of_overlays(std::vector<overlay_info> &overlays)
{
    std::vector<cv::Size> sizes;
    for (auto &overlay : overlays)
    {
        sizes.push_back(overlay.image.size());
    }
    return sizes;
}

std::vector<cv::Size> size_overlays(std::vector<overlay_info> &overlays)
{
    std::vector<cv::Size> sizes;
    for (auto &overlay : overlays)
    {
        sizes.push_back(overlay.image.size());
    }
    return sizes;
}

void tranf_overlays(std::vector<overlay_info> &overlays)
{
    const double image_scale = 2;
    const double map_scale = 1.5;
    for (auto &overlay : overlays)
    {
        // rgba_to_rgb(overlay.image);
        cv::resize(overlay.image, overlay.image, cv::Size(), image_scale, image_scale);
        overlay.tl *= map_scale;
        overlay.br *= map_scale;
    }
}