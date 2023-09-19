#pragma once
#include <opencv2/xfeatures2d.hpp>
#include <meojson/include/json5.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iostream>

#include "../../source/tianli.core/core.map/map.combiner.h"
#include "../../source/tianli.utils/utils.operation.image.h"
#include "../../source/tianli.utils/utils.convect.string.h"

struct city_info
{
    int area_id;
    std::string name;
    std::string path;
    cv::Mat image;
    cv::Mat image_resize;
    cv::Rect area_rect;
    cv::Size tile_size;
    double zoom = 0;
    cv::Rect rect() { return cv::Rect(area_rect.tl(), cv::Size(area_rect.width * 1.5, area_rect.height * 1.5)); }
};

std::vector<city_info> from_json_city(std::string json_file)
{
    std::vector<city_info> citys;
    std::ifstream in(json_file);
    std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto json_res = json::parse5(str);
    if (json_res.has_value() == false)
    {
        return citys;
    }
    auto json = json_res.value();
    auto type = json.type();
    for (auto &other_map : json["other_map"].as_array())
    {
        for (auto &area : other_map["areas"].as_array())
        {
            auto area_id = area["area_id"].as_integer();
            auto name = area["name"].as_string();
            auto path = area["path"].as_string();
            auto area_rect = area["area_rect"].as_array();
            auto x = area_rect[0].as_integer();
            auto y = area_rect[1].as_integer();
            auto x2 = area_rect[2].as_integer();
            auto y2 = area_rect[3].as_integer();
            auto tile_size = area["tile_size"].as_array();
            auto x1 = tile_size[0].as_integer();
            auto y1 = tile_size[1].as_integer();
            auto zoom = area["zoom"].as_double();
            cv::Mat image;
            if (std::filesystem::exists(std::filesystem::path("./city") / path))
            {
                image = cv::imread((std::filesystem::path("./city") / path).string(), -1);
                citys.push_back({area_id, name, path, image, cv::Mat(), cv::Rect(x, y, x2 - x, y2 - y), cv::Size(x1, y1), zoom});
            }
            else
            {
                std::cout << path << " not exists" << std::endl;
            }
            std::cout << utils::utf8_to_gbk(area["name"].as_string()) << std::endl;
        }
    }
    return citys;
}

std::vector<cv::Size> size_of_citys(std::vector<city_info> &citys)
{
    std::vector<cv::Size> sizes;
    for (auto &city : citys)
    {
        sizes.push_back(city.image_resize.size());
    }
    return sizes;
}

std::vector<city_info> tranf_citys(std::vector<city_info> &citys)
{
    const double image_scale = 2;
    for (auto &city : citys)
    {
        // rgba_to_rgb(city.image);
        cv::resize(city.image, city.image_resize, cv::Size(), image_scale, image_scale);
    }
    return citys;
}