#pragma once
#include <map>
#include <vector>
#include <list>
#include <future>
#include <opencv2/opencv.hpp>

class obj
{
    cv::Size size;
    int priority = 0;

public:
    enum binpry_base : int
    {
        priority_base = 0x00,
        align_flipped = 0x01,
        rotate_topple = 0x02,
        priority_mask = 0xf0,
    };
    enum binpry_place : int
    {
        WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_GT_DROP_H = 0x60,
        WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_EQ_DROP_H = 0x80,
        WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_BTW_DROPS = 0x40,
        WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_EQ_DROP_L = 0x70,
        WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_LT_DROP_L = 0x20,
        WIDTH_LT_SL_BOTTOM_AND_HEIGHT_EQ_DROP_H = 0x50,
        WIDTH_LT_SL_BOTTOM_AND_HEIGHT_EQ_DROP_L = 0x31,
        WIDTH_LT_SL_BOTTOM_AND_HEIGHT_NE_ANYDRP = 0x10,
        BIN_PACKED_PLACEMENT_PRIORITY_UNDEFINED = 0x00,
    };

public:
    obj(int width, int height) : size(width, height) {}
    int width() const { return size.width; }
    int height() const { return size.height; }
    int weight() const { return width() * height(); }
    int get_priority() const { return priority; }
    bool better_than(std::list<obj> &objs, std::list<obj>::iterator it) const
    {
        if (it == objs.end())
            return true;
        int cmp = compare(priority, it->priority);
        return cmp > 0 || (cmp == 0 && weight() > it->weight());
    }
    bool is_valid() const { return 0 != (priority & priority_mask); }
    int judge(int btm, int left, int right, int limit)
    {
        if (left <= right)
        {
            priority = priority_base ^ judge_all(btm, left, right, limit, width(), height());
        }
        else
        {
            priority = priority_base ^ judge_all(btm, right, left, limit, height(), width()) ^ align_flipped;
        }
        return priority;
    }

private:
    static int judge_one(int btm, int low, int high, int limit, int obj_width, int obj_height)
    {

        if (obj_height <= limit)
        {
            if (btm == obj_width)
            {
                if (obj_height > high)
                    return binpry_place::WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_GT_DROP_H;
                if (obj_height == high)
                    return binpry_place::WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_EQ_DROP_H;
                if (obj_height > low)
                    return binpry_place::WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_BTW_DROPS;
                if (obj_height == low)
                    return binpry_place::WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_EQ_DROP_L;
                return binpry_place::WIDTH_EQ_SL_BOTTOM_AND_HEIGHT_LT_DROP_L;
            }
            else if (btm > obj_width)
            {
                if (obj_height == high)
                    return binpry_place::WIDTH_LT_SL_BOTTOM_AND_HEIGHT_EQ_DROP_H;
                if (obj_height == low)
                    return binpry_place::WIDTH_LT_SL_BOTTOM_AND_HEIGHT_EQ_DROP_L;
                return binpry_place::WIDTH_LT_SL_BOTTOM_AND_HEIGHT_NE_ANYDRP;
            }
        }
        return binpry_place::BIN_PACKED_PLACEMENT_PRIORITY_UNDEFINED;
    }
    static int judge_all(int btm, int low, int high, int limit, int obj_width, int obj_height)
    {
        return judge_one(btm, low, high, limit, obj_width, obj_height);
        // int p1 = judge_one(btm, low, high, limit, obj_width, obj_height);
        // int p2 = judge_one(btm, low, high, limit, obj_height, obj_width) ^ binpry_base::rotate_topple;
        // return compare(p1, p2) < 0 ? p2 : p1;
    }
    static int compare(const int &pry_a, const int &pry_b)
    {
        return (priority_mask & pry_a) - (priority_mask & pry_b);
    }
};

struct placement
{
    cv::Rect rect;
    bool is_toppled = false;
};

class skyline
{
public:
    skyline(int start, int width, int level) : start(start), width(width), level(level) {}
    ~skyline() = default;
    int start = 0;
    int width = 0;
    int level = 0;

public:
    enum step_adjoin : int
    {
        undefined = 0,
        reverse = -1,
        obverse = 1,
    };
    static int adjoin(skyline *step_a, skyline *step_b)
    {
        if (step_a && step_b)
        {
            if (step_a->start + step_a->width == step_b->start)
                return obverse;
            if (step_b->start + step_b->width == step_a->start)
                return reverse;
        }
        return undefined;
    }

    static std::list<skyline>::iterator insert_to(std::list<skyline> &steps, skyline *step)
    {
        for (auto it = steps.begin(); it != steps.end(); it++)
        {
            if ((it->level > step->level) || (it->level == step->level && it->start >= step->start))
            {
                return steps.insert(it, *step);
            }
        }
        // queue.PushTail(pTar);
        return steps.insert(steps.end(), *step);
    }
    // SkylineStepT::insertTo(queStep, queStep.Dechain(pCurStep));
    static void insert_to_dechain(std::list<skyline> &steps, std::list<skyline>::iterator &it)
    {
        skyline tmp = *it;
        steps.erase(it);
        it = insert_to(steps, &tmp);
    }

    void measure_both_flank(std::list<skyline> &steps, std::list<skyline>::iterator it, int &drop_left, int &drop_right, const int &drop_top)
    {
        drop_left = drop_right = drop_top;
        auto cur_step = it++;
        int cnt = 2;
        while ((cur_step != steps.end()) && (cnt > 0))
        {
            switch (adjoin(&*cur_step, this))
            {
            case step_adjoin::obverse:
                drop_left = cur_step->level - level;
                break;
            case step_adjoin::reverse:
                drop_right = cur_step->level - level;
                break;
            default:
                --cnt;
                break;
            }
            cur_step++;
        }
    }
};

class combiner_inface
{
public:
    combiner_inface() = default;

public:
    virtual void init(std::vector<cv::Size> &objs) = 0;
    virtual std::map<int, cv::Rect> pre_combinered(cv::Size limit_size) = 0;
};
class combine : public combiner_inface
{
public:
    combine() = default;
    ~combine() = default;

public:
    void add(int width, int height) { objs.emplace_back(width, height); }

    void init(std::vector<cv::Size> &objs)
    {
        for (auto &obj : objs)
        {
            add(obj.width, obj.height);
        }
    }
    virtual std::map<int, cv::Rect> pre_combinered(cv::Size limit_size)
    {
        std::list<skyline> steps;
        std::list<placement> places;
        auto head = pack(limit_size.width, limit_size.height, steps, places);
        std::map<int, cv::Rect> result;
        int index = 0;
        for (auto &place : places)
        {
            result[index++] = place.rect;
        }
        return result;
    }

private:
    obj *pack(int width, int height, std::list<skyline> &steps, std::list<placement> &places)
    {
        auto head = objs.begin();
        skyline::insert_to(steps, new skyline(0, width, 0));
        while (head != objs.end())
        {
            auto cur_step = steps.begin();
            auto tmp_step = steps.end();
            auto __cur_step = cur_step;
            while (tmp_step = __cur_step++, tmp_step != steps.end())
            {
                if ((cur_step->level != tmp_step->level) ||
                    (skyline::adjoin(&*cur_step, &*tmp_step) == skyline::step_adjoin::undefined))
                {
                    break;
                }
                cur_step->width = cur_step->width + tmp_step->width;
                steps.erase(tmp_step);
            }

            int drop_m = height - cur_step->level;
            int drop_left = 0;
            int drop_right = 0;
            cur_step->measure_both_flank(steps, cur_step, drop_left, drop_right, drop_m);

            auto cur_obj = head;
            auto tar_obj = objs.end();
            for (; cur_obj != objs.end(); cur_obj++)
            {
                if (cur_obj->judge(cur_step->width, drop_left, drop_right, drop_m), cur_obj->is_valid() && cur_obj->better_than(objs, tar_obj))
                {
                    tar_obj = cur_obj;
                }
            }
            __cur_step = cur_step;
            if (tar_obj != objs.end())
            {
                bool toppled = (tar_obj->get_priority() & obj::binpry_base::rotate_topple) == obj::binpry_base::rotate_topple;
                bool flipped = (tar_obj->get_priority() & obj::binpry_base::align_flipped) == obj::binpry_base::align_flipped;
                int bottom = toppled ? tar_obj->height() : tar_obj->width();
                int old_level = cur_step->level;
                int tar_level = cur_step->level + (toppled ? tar_obj->width() : tar_obj->height());
                int remain = cur_step->width - bottom;
                if (remain != 0)
                {
                    auto *new_step = new skyline(cur_step->start, remain, old_level);
                    if (flipped)
                    {
                        cur_step->start = cur_step->start + remain;
                    }
                    else
                    {
                        new_step->start = cur_step->start + bottom;
                    }
                    skyline::insert_to(steps, new_step);
                }
                cur_step->level = tar_level;
                cur_step->width = bottom;

                auto step = *cur_step;
                skyline::insert_to_dechain(steps, cur_step);
                // steps.erase(cur_step);

                if (head == tar_obj)
                {
                    head++;
                }
                // move to head
                auto tar = *tar_obj;
                objs.erase(tar_obj);
                // objs.emplace_front(tar);
                // tar_obj = objs.begin();
                placement place;
                // place.rect = cv::Rect(cur_step->start, old_level, bottom, tar_obj->height());
                place.rect.x = step.start;
                place.rect.y = old_level;
                place.rect.width = bottom;
                place.rect.height = tar.height(); // tar_obj->height();
                place.is_toppled = toppled;
                // places.push_back(placement{cv::Rect(cur_step->start, old_level, bottom, tar_obj->height()), toppled});
                places.push_back(place);
            }
            else if (__cur_step++, __cur_step != steps.end())
            {
                cur_step->level += (drop_left > drop_right ? drop_right : drop_left);
                // skyline::insert_to_dechain(steps, cur_step);
                steps.erase(cur_step);
            }
            else
            {
                break;
            }
        }
        if (head != objs.end())
            return &*head;
        return nullptr;
    }

private:
    std::list<obj> objs;
};

void test()
{
    std::vector<cv::Size> objs = {
        {416, 432},
        {416, 400},
        {616, 496},
        {784, 560},
        {152, 144},
        {3168, 3352},
        {1912, 3184},
        {520, 368},
        {656, 544},
        {488, 1312},
        {568, 792},
        {384, 304},
        {376, 696},
        {1064, 1552},
        {392, 568},
        {104, 56},
        {632, 760},
        {840, 544},
        {280, 344},
        {560, 440},
        {600, 648},
        {256, 256},
        {600, 440},
        {432, 416},
        {392, 488},
        {1048, 984},
        {448, 464},
        {296, 296},
        {360, 496},
        {136, 112},
        {216, 112},
        {784, 1696},
        {344, 400},
        {144, 192},
        {512, 360},
        {256, 232},
        {664, 912},
        {472, 776},
        {280, 360},
        {376, 312},
        {416, 760},
        {1176, 1128},
        {608, 816},
        {528, 680},
        {264, 672},
        {504, 392},
        {272, 336},
        {440, 320},
        {600, 360},
        {480, 616},
        {480, 360},
        {536, 128},
        {248, 552},
        {560, 512},
        {1056, 992},
        {432, 760},
        {248, 408},
        {352, 504},
        {344, 256},
        {328, 272},
        {296, 536},
        {352, 512},
        {392, 448},
        {680, 680},
        {856, 696},
        {1360, 1376},
        {632, 648},
        {360, 416},
        {952, 880},
        {1472, 1504},
        {520, 688},
        {696, 752},
        {1336, 968},
        {1240, 808},
        {872, 456},
        {768, 1016},
        {384, 312},
        {936, 632},
        {424, 416},
        {328, 328},
        {568, 720},
        {472, 744},
        {656, 600},
        {448, 416},
        {584, 456},
        {704, 688},
        {816, 776},
        {640, 864},
        {592, 296},
        {536, 504},
        {472, 600},
        {720, 616},
        {504, 584},
        {648, 520},
        {488, 440},
        {448, 632},
        {664, 400},
        {776, 608},
        {784, 416},
        {584, 600},
        {632, 336},
        {872, 768},
        {488, 696}};
    std::sort(objs.begin(), objs.end(), [](const cv::Size &a, const cv::Size &b)
              { return a.width * a.height > b.width * b.height; });

    cv::Size limit = {2048 * 6, 2048 * 4};

    combiner_inface *combiner = new combine();
    combiner->init(objs);
    auto result = combiner->pre_combinered(limit);
    std::vector<cv::Rect> rects;
    cv::Rect max_rect;
    for (auto &r : result)
    {
        rects.push_back(r.second);
        max_rect = max_rect | r.second;
        std::cout << r.first << ": " << r.second << std::endl;
    }
    cv::Mat img = cv::Mat::zeros(max_rect.size(), CV_8UC3);
    for (auto &r : rects)
    {
        cv::rectangle(img, r, cv::Scalar(0, 0, 255), 10);
    }
    cv::resize(img, img, cv::Size(), 0.1, 0.1);
    cv::imshow("img", img);
    cv::waitKey(000);
}