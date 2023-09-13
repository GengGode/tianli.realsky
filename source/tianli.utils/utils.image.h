#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>
namespace utils
{
    inline QImage float_mat_to_qimage(const cv::Mat &float_src)
    {
        if (float_src.empty())
            return QImage();
        QImage dest;
        cv::Mat src;
        float_src.convertTo(src, CV_8U);

        if (src.channels() == 4)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_ARGB32);
        else if (src.channels() == 3)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_RGB888);
        else if (src.channels() == 1)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_Grayscale8);
        else
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_ARGB32);
        dest.bits(); // enforce deep copy, see documentation
        // of QImage::QImage ( const uchar * data, int width, int height, Format format )
        return dest;
    }
    inline QImage mat_to_qimage(const cv::Mat &src)
    {
        if (src.empty())
            return QImage();
        QImage dest;
        if (src.channels() == 4)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_ARGB32);
        else if (src.channels() == 3)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_RGB888);
        else if (src.channels() == 1)
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_Grayscale8);
        else
            dest = QImage((const uchar *)(src.data), src.cols, src.rows, src.cols * (src.channels()), QImage::Format_ARGB32);
        dest.bits(); // enforce deep copy, see documentation
        // of QImage::QImage ( const uchar * data, int width, int height, Format format )
        return dest;
    }
    inline cv::Mat qimage_to_mat(const QImage &src)
    {
        if (src.isNull())
            return cv::Mat();
        cv::Mat tmp(src.height(), src.width(), CV_MAKETYPE(CV_8U, src.depth() / 8), (uchar *)src.bits(), src.bytesPerLine());
        cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
        result = tmp.clone();
        return result;
    }

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
}