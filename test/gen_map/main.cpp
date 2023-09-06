#include <iostream>
#include "BlockMapResource.h"
#include "MapItemSet.h"
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

void rgba_to_rgb(cv::Mat &image)
{
    if (image.channels() == 4)
    {
        // cvtColor 会直接丢弃 alpha 通道，所以需要手动混合
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        cv::Mat alpha = channels[3];
        channels[0] = channels[0].mul(alpha) / 255;
        channels[1] = channels[1].mul(alpha) / 255;
        channels[2] = channels[2].mul(alpha) / 255;
        channels.pop_back();
        cv::merge(channels, image);
    }
}

void rbg_add_rgba(cv::Mat &image, cv::Mat &overlay)
{
    if (image.size() != overlay.size())
        return;
    if (overlay.channels() == 4)
    {
        // 根据 overlay alpha 通道混合
        std::vector<cv::Mat> image_channels;
        std::vector<cv::Mat> overlay_channels;
        cv::split(image, image_channels);
        cv::split(overlay, overlay_channels);
        cv::Mat alpha = overlay_channels[3];
        image_channels[0] = image_channels[0].mul(255 - alpha) / 255 + overlay_channels[0].mul(alpha) / 255;
        image_channels[1] = image_channels[1].mul(255 - alpha) / 255 + overlay_channels[1].mul(alpha) / 255;
        image_channels[2] = image_channels[2].mul(255 - alpha) / 255 + overlay_channels[2].mul(alpha) / 255;
        cv::merge(image_channels, image);
    }
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

#include "OverlaySort.h"

#include <opencv2/core/utils/logger.hpp>
int main(int argc, char *argv[])
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

    auto overlays = from_json_overlay("./overlay/web-map.json");
    tranf_overlays(overlays);

    test();

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

    auto points = from_txt();
    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < points.size(); i++)
        items.push_back(std::make_shared<ItemObject>(points[i], "name" + std::to_string(i)));

    BlockMapResource quadTree("./map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    auto map_center = quadTree.get_abs_origin();
    auto r2 = map_center - quadTree.get_min_rect().tl();
    auto r = r2 + cv::Point(-6465, -6622);
    auto r3 = r2 + cv::Point(-6465, -6622) * 1.5;
    std::cout << r2.x << ", " << r2.y << std::endl;
    std::cout << r.x << ", " << r.y << std::endl;
    std::cout << r3.x << ", " << r3.y << std::endl;

    auto m = cv::imread("C:/Users/XiZhu/source/repos/tianli.RealSky/test/gen_map/overlay/Tex_0232_0~6.png", -1);
    cv::resize(m, m, cv::Size(), 2, 2);

    auto map_a = quadTree.view();
    for (auto &overlay : overlays)
    {
        // map_a(quadTree.to_abs(overlay.rect())).copyTo();
        std::cout << overlay.rect() << quadTree.abs(overlay.rect()) << std::endl;
        auto map_bound = map_a(quadTree.abs(overlay.rect()));
        rbg_add_rgba(map_bound, overlay.image);
        // overlay.image.copyTo(map_a(quadTree.abs(overlay.rect())));
    }
    cv::imwrite("map_a.png", map_a);

    auto map = quadTree.view(cv::Rect2d(cv::Point(-6465, -6622) * 1.5, m.size()));

    cv::cvtColor(m, m, cv::COLOR_BGRA2BGR);
    cv::addWeighted(map, 0.5, m, 0.5, 0, map);

    auto max_rect = get_max_rect(quadTree); // cv::Rect2d(quadTree.get_min_rect());
    auto origin = cv::Rect2d(quadTree.get_min_rect()).tl() - cv::Point2d(map_center);

    ItemSetTree tree(max_rect, items);
    {
        track_timer timer("find rect(2000,2000)");
        auto result = tree.find_childs(cv::Rect2d(-1000, -1000, 2000, 2000));
        auto map = quadTree.view(cv::Rect2d(-1000, -1000, 2000, 2000));
        for (auto &node : result)
        {
            auto scale = 1.5;
            for (auto &item : node->items)
            {
                auto item_pos = item->pos() * scale - cv::Point2d(-1000, -1000);
                cv::circle(map, item_pos, 10, cv::Scalar(0, 0, 255), 4);
            }
            auto node_rect = cv::Rect2d(node->rect.tl() * scale - cv::Point2d(-1000, -1000), cv::Size2d(node->rect.width * scale, node->rect.height * scale));
            cv::rectangle(map, node_rect, cv::Scalar(0, 255, 0), 1);
        }
    }
    {
        track_timer timer("find max_rect");
        auto result = tree.find_childs(max_rect);
        for (auto &node : result)
        {
            auto scale = 1.5;
            for (auto &item : node->items)
            {
                auto item_pos = item->pos() * scale - origin;
                cv::circle(map, item_pos, 10, cv::Scalar(0, 0, 255), 4);
            }
            auto node_rect = cv::Rect2d(node->rect.tl() * scale - origin, cv::Size2d(node->rect.width * scale, node->rect.height * scale));
            cv::rectangle(map, node_rect, cv::Scalar(0, 255, 0), 1);
        }
    }
    {

        track_timer timer("asd");
        auto map = quadTree.view(cv::Rect2d(-1000, -1000, 2000, 2000));
    }

    // gen_surf(quadTree);

    return 0;
}