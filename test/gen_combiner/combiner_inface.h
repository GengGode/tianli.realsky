#pragma once
#include <map>
#include <vector>
#include <list>

struct rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};
struct size
{
    int width = 0;
    int height = 0;
};

struct pre_combinered
{
    std::map<int, rect> places;
};

class combiner_inface
{
public:
    combiner_inface() = default;

public:
    virtual void init(std::vector<size> &objs) = 0;
    virtual std::map<int, rect> pre_combinered(size limit_size) = 0;
};
