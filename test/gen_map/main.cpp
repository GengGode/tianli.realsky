#include <iostream>
#include "BlockMapResource.h"
#include "MapItemSet.h"

#include <opencv2/xfeatures2d.hpp>

#include <meojson/include/json.hpp>
std::vector<cv::Point2d> from_json(std::string json_file);
std::vector<cv::Point2d> from_txt()
{
    std::vector<cv::Point2d> points;
    std::vector<std::string> paths = {
        "../../src/item/json_file_0.json",
        "../../src/item/json_file_1.json",
        "../../src/item/json_file_2.json",
        "../../src/item/json_file_3.json",
        "../../src/item/json_file_4.json",
        "../../src/item/json_file_5.json",
        "../../src/item/json_file_6.json",
        "../../src/item/json_file_7.json",
        "../../src/item/json_file_8.json",
        "../../src/item/json_file_9.json",
        "../../src/item/json_file_10.json",
        "../../src/item/json_file_11.json",
        "../../src/item/json_file_12.json",
        "../../src/item/json_file_13.json",
        "../../src/item/json_file_14.json",
        "../../src/item/json_file_15.json",
        "../../src/item/json_file_16.json",
        "../../src/item/json_file_17.json",
        "../../src/item/json_file_18.json",
        "../../src/item/json_file_19.json"};
    for (auto &path : paths)
    {
        auto point = from_json(path);
        // merge
        points.insert(points.end(), point.begin(), point.end());
    }

    return points;
}
#include <fstream>
std::vector<cv::Point2d> from_json(std::string json_file)
{
    std::vector<cv::Point2d> points;
    std::ifstream in(json_file);
    std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto json_res = json::parse(str);
    if (json_res.has_value() == false)
    {
        return points;
    }
    auto json = json_res.value();
    auto type = json.type();
    auto array = json.as_array();
    for (auto &item : array)
    {
        auto position_str = item["position"].as_string();
        // std::cout << position_str << std::endl;
        //  350.20,-456.00
        double x = std::stod(position_str.substr(0, position_str.find(",")));
        double y = std::stod(position_str.substr(position_str.find(",") + 1));
        points.push_back(cv::Point2d(x, y));
    }

    return points;
}

cv::Rect2d get_max_rect(BlockMapResource &quadTree)
{
    auto map_center = quadTree.get_abs_origin();
    auto map_rect = quadTree.get_min_rect();
    // 根据地图的中心点和地图的最小矩形，分割为四个矩形
    auto map_rect_top_left = cv::Rect2d(map_rect.x, map_rect.y, map_center.x - map_rect.x, map_center.y - map_rect.y);
    auto map_rect_top_right = cv::Rect2d(map_center.x, map_rect.y, map_rect.x + map_rect.width - map_center.x, map_center.y - map_rect.y);
    auto map_rect_bottom_left = cv::Rect2d(map_rect.x, map_center.y, map_center.x - map_rect.x, map_rect.y + map_rect.height - map_center.y);
    auto map_rect_bottom_right = cv::Rect2d(map_center.x, map_center.y, map_rect.x + map_rect.width - map_center.x, map_rect.y + map_rect.height - map_center.y);
    // 获取最大半径
    std::vector<double> rect_radius = {map_rect_top_left.width, map_rect_top_left.height, map_rect_top_right.width, map_rect_top_right.height, map_rect_bottom_left.width, map_rect_bottom_left.height, map_rect_bottom_right.width, map_rect_bottom_right.height};
    auto max_radius = *std::max_element(rect_radius.begin(), rect_radius.end());
    int max_radius_int = static_cast<int>(std::round(max_radius));
    return cv::Rect2d(-max_radius_int, -max_radius_int, max_radius_int * 2, max_radius_int * 2);
}
int main(int argc, char *argv[])
{

    {
        // fs read
        cv::FileStorage fs2("surf.xml", cv::FileStorage::READ);
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        fs2["keypoints"] >> keypoints;
        fs2["descriptors"] >> descriptors;
        fs2.release();
        std::vector<std::shared_ptr<ItemInface>> items;
        for (int i = 0; i < keypoints.size(); i++)
            items.push_back(std::make_shared<ItemObject>(keypoints[i].pt, "name" + std::to_string(i)));
        ItemSetTree tree(cv::Rect(0, 0, 40000, 40000), items);

        auto res = tree.find(tree.root->rect);

        tree.print();
    }

    auto points = from_txt();
    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < points.size(); i++)
        items.push_back(std::make_shared<ItemObject>(points[i], "name" + std::to_string(i)));

    BlockMapResource quadTree("../../src/map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    auto map_center = quadTree.get_abs_origin();
    auto max_rect = get_max_rect(quadTree); // cv::Rect2d(quadTree.get_min_rect());
    auto origin = cv::Rect2d(quadTree.get_min_rect()).tl() - cv::Point2d(map_center);

    ItemSetTree tree(max_rect, items);
    {
        auto result = tree.find_childs(cv::Rect2d(-1000, -1000, 2000, 2000));
        auto map = quadTree.view(cv::Rect2d(-1000, -1000, 2000, 2000));
        for (auto &node : result)
        {
            auto scale = 1.5;
            for (auto &item : node->items)
            {
                auto item_pos = item->pos * scale - cv::Point2d(-1000, -1000);
                cv::circle(map, item_pos, 10, cv::Scalar(0, 0, 255), 4);
            }
            auto node_rect = cv::Rect2d(node->rect.tl() * scale - cv::Point2d(-1000, -1000), cv::Size2d(node->rect.width * scale, node->rect.height * scale));
            cv::rectangle(map, node_rect, cv::Scalar(0, 255, 0), 1);
        }
    }
    {
        auto result = tree.find_childs(max_rect);
        auto map = quadTree.view();
        for (auto &node : result)
        {
            auto scale = 1.5;
            for (auto &item : node->items)
            {
                auto item_pos = item->pos * scale - origin;
                cv::circle(map, item_pos, 10, cv::Scalar(0, 0, 255), 4);
            }
            auto node_rect = cv::Rect2d(node->rect.tl() * scale - origin, cv::Size2d(node->rect.width * scale, node->rect.height * scale));
            cv::rectangle(map, node_rect, cv::Scalar(0, 255, 0), 1);
        }
    }
    {
        auto map = quadTree.view();
        // surf key
        cv::Ptr<cv::xfeatures2d::SURF> surf = cv::xfeatures2d::SURF::create(400);
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        surf->detectAndCompute(map, cv::Mat(), keypoints, descriptors);
        // fs write
        cv::FileStorage fs("surf.xml", cv::FileStorage::WRITE);
        fs << "keypoints" << keypoints;
        fs << "descriptors" << descriptors;
        fs.release();
    }
    return 0;
}