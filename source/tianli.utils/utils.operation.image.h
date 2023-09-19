#pragma once
#include <opencv2/opencv.hpp>
namespace utils
{
    inline void add_rgba_image(cv::Mat &src1, cv::Mat &src2, cv::Mat &dst, double alpha)
    {
        assert(src1.channels() == 4 && src2.channels() == 4);

        std::vector<cv::Mat> src1_split;
        std::vector<cv::Mat> src2_split;
        std::vector<cv::Mat> dst_merge;

        cv::split(src1, src1_split);
        cv::split(src2, src2_split);

        for (int i = 0; i < 3; i++)
        {
            auto dst_src1 = src1_split[i].mul(~src2_split[3], 1.0 / 255.0);
            auto dst_src2 = src2_split[i].mul(src2_split[3], alpha / 255.0);
            auto dst_channel = dst_src1 + dst_src2;
            dst_merge.push_back(dst_channel);
        }
        cv::Mat alpha_dst = src1_split[3] + src2_split[3] * alpha;
        dst_merge.push_back(alpha_dst);
        cv::merge(dst_merge, dst);
    }

    inline void rgba_to_rgb(cv::Mat &image)
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

    inline void rbg_add_rgba(cv::Mat &image, cv::Mat &overlay)
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

}