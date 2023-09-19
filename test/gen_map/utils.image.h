#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

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
    else if (overlay.channels() == 3)
    {
        // 直接覆盖
        overlay.copyTo(image);
    }
}
