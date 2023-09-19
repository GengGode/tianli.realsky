#include <iostream>
#include "BlockMapResource.h"
#include "MapItemSet.h"
#include "MapOverlay.h"
#include "MapCity.h"
#include <direct.h> // add this header file

#include <opencv2/xfeatures2d.hpp>

#include <meojson/include/json.hpp>
#include <filesystem>
#include <chrono>
class track_timer
{
public:
    track_timer(std::string name = "null") : start_(std::chrono::high_resolution_clock::now()), name_(name)
    {
        std::cout << "[" << name_ << "] begin -" << std::endl;
    }
    ~track_timer() { std::cout << "[" << name_ << "] end : " << elapsed() << std::endl; }
    std::chrono::microseconds elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_);
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    std::string name_;
};

std::vector<cv::Point2d> from_json(std::string json_file);
std::vector<cv::Point2d> from_txt()
{
    // cout current path
    char buf[256];
    _getcwd(buf, 256); // use _getcwd instead of getcwd
    std::cout << buf << std::endl;
    std::vector<cv::Point2d> points;
    std::filesystem::path path(buf);
    if (std::filesystem::exists(path / "save") == false)
    {
        std::cout << "item not exists" << std::endl;
        return points;
    }

    std::vector<std::string> paths;
    for (auto &p : std::filesystem::directory_iterator(path / "save"))
    {
        if (p.path().extension() == ".json")
        {
            paths.push_back(p.path().string());
        }
    }

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

void gen_surf(BlockMapResource &quadTree)
{

    track_timer timer;
    auto map = quadTree.view();
    // save map
    cv::imwrite("map.png", map);
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
void test_surf_gen()
{
    auto map = cv::imread("map/UI_MapBack_-1_0.png");
    cv::Ptr<cv::xfeatures2d::SURF> surf = cv::xfeatures2d::SURF::create(400);
    cv::Rect2d rand_rect; // = cv::Rect2d(-1000, -1000, 400, 400);
    track_timer timer(__FUNCTION__);
    for (int i = 0; i < 100; i++)
    {
        rand_rect.x = 0; // rand() % 4000 - 2000;
        rand_rect.y = 0; // rand() % 4000 - 2000;
        rand_rect.width = 400;
        rand_rect.height = 400;
        // surf key
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        // 计算关键点和描述子
        surf->detectAndCompute(map(rand_rect), cv::Mat(), keypoints, descriptors);
    }
    auto time = timer.elapsed() / 100.0;
    std::cout << "surf gen avg time : " << time << std::endl;
}

void test_tree_find(ItemSetTree &tree, cv::Mat &descriptors)
{
    cv::Rect2d rand_rect; // = cv::Rect2d(-1000, -1000, 400, 400);

    track_timer timer(__FUNCTION__);
    for (int i = 0; i < 10000; i++)
    {
        rand_rect.x = 16384 - 1024 + 232; // rand() % 4000 - 2000;
        rand_rect.y = 12288 - 1024 + 216; // rand() % 4000 - 2000;
        rand_rect.width = 400;
        rand_rect.height = 400;
        // track_timer timer("find");
        auto result = tree.find(rand_rect);
        std::vector<cv::KeyPoint> res_keypoints(result.size());
        cv::Mat res_descriptor(result.size(), 64, CV_32F);
        int index = 0;
        for (const auto &item : result)
        {
            auto keypoint_ptr = std::dynamic_pointer_cast<KeyPointObject>(item);
            // 组合关键点
            res_keypoints[index] = *keypoint_ptr->kp;
            // 组合描述子
            auto descriptor = descriptors.row(keypoint_ptr->index);
            descriptor.copyTo(res_descriptor.row(index));
            index++;
        }
    }
    auto time = timer.elapsed() / 10000.0;
    std::cout << "find avg time : " << time << std::endl;
}

/*
    {
        // fs read
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        {
            track_timer timer("read surf.xml");
            cv::FileStorage fs2("surf.xml", cv::FileStorage::READ);
            fs2["keypoints"] >> keypoints;
            fs2["descriptors"] >> descriptors;
            fs2.release();
        }
        std::vector<std::shared_ptr<ItemInface>> items;
        for (int i = 0; i < keypoints.size(); i++)
            items.push_back(std::make_shared<KeyPointObject>(keypoints[i], i));
        ItemSetTree tree(cv::Rect(0, 0, 40000, 40000), items);

        auto res = tree.find(tree.root->rect);

        test_tree_find(tree, descriptors);
        test_surf_gen();
        tree.print();
    }
*/

void add_items()
{
    auto points = from_txt();
    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < points.size(); i++)
        items.push_back(std::make_shared<ItemObject>(points[i], "name" + std::to_string(i)));
}

void add_overlay(BlockMapResource &quadTree, cv::Mat &map, std::string &overlays_json)
{
    auto overlays = from_json_overlay(overlays_json);
    tranf_overlays(overlays);
    auto sizes = size_of_overlays(overlays);
    auto ref_map_rect = cv::Rect(0, map.rows - 2048 * 3, 2048 * 6, 2048 * 3);
    auto map_black = map(ref_map_rect);
    auto res = pack(sizes, ref_map_rect.size());
    for (auto &overlay : overlays)
    {
        auto map_bound = map(quadTree.abs(overlay.rect()));
        rbg_add_rgba(map_bound, overlay.image);
        if (res.find(overlay.image.size()) != res.end())
        {
            auto rect = res[overlay.image.size()];
            auto map_bound = map_black(rect);
            rbg_add_rgba(map_bound, overlay.image);
        }
        else
        {
            std::cout << "not found" << std::endl;
        }
    }
}

void add_city(BlockMapResource &quadTree, cv::Mat &map, std::string &citys_json)
{
    auto citys = from_json_city(citys_json);
    // citys = tranf_citys(citys);
    for (auto &city : citys)
    {
        auto rect = quadTree.abs(cv::Rect(city.rect().tl() * 1.5, city.rect().size()));
        auto city_map = map(rect);
        cv::resize(city_map, city.image_resize, cv::Size(), 2, 2);

        cv::imwrite("city/" + utf8_to_gbk(city.name) + ".png", city.image_resize);
    }
    auto sizes = size_of_citys(citys);

    auto ref_map_rect = cv::Rect(map.cols - 2048 * 5, 0, 2048 * 5, 2048 * 2);
    auto map_black = map(ref_map_rect);
    auto res = pack(sizes, ref_map_rect.size());
    for (auto &city : citys)
    {
        if (res.find(city.image_resize.size()) != res.end())
        {
            auto rect = res[city.image_resize.size()];
            auto map_bound = map_black(rect);
            rbg_add_rgba(map_bound, city.image_resize);
        }
        else
        {
            std::cout << "not found" << std::endl;
        }
    }

    json::value json;
    json["city_ref"] = json::array();
    for (auto &city : citys)
    {
        if (res.find(city.image_resize.size()) != res.end())
        {
            auto ref_rect = quadTree.abs(cv::Rect(city.rect().tl() * 1.5, city.rect().size()));
            auto map_rect = res[city.image_resize.size()] + ref_map_rect.tl();
            json["city_ref"].as_array().push_back(json::object{
                {"name", city.name},
                {"path", city.path},
                {"map_rect", json::array{map_rect.x, map_rect.y, map_rect.width, map_rect.height}},
                {"map_ref_rect", json::array{ref_rect.x, ref_rect.y, ref_rect.width, ref_rect.height}},
                {"ref_to_rect_scale", 2.0},
                {"base_size", 2048}});
        }
    }
    cv::imwrite("mapcity.png", map_black);
    std::ofstream out("city_ref.json");
    out << json;
    out.close();
}

#include <opencv2/core/utils/logger.hpp>
int main(int argc, char *argv[])
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

    std::string overlays_json = "./overlay/web-map.json";
    std::string citys_json = "./city/citys.json5";

    //
    BlockMapResource quadTree("./map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    //
    auto map = quadTree.view();
    add_city(quadTree, map, citys_json);
    cv::imwrite("map_b.png", map);
    add_overlay(quadTree, map, overlays_json);
    cv::imwrite("map_a.png", map);

    return 0;
}