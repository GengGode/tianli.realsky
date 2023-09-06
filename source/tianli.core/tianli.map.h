#pragma once

#include "core.map/item.set.h"
#include "core.map/map.resource.h"

class Map
{
public:
    Map()
    {
        // random
        std::vector<std::shared_ptr<ItemInface>> items;
        for (int i = 0; i < 100; i++)
        {
            items.push_back(std::make_shared<ItemObject>(cv::Point2d(rand() % 1000, rand() % 1000), std::to_string(i)));
        }
        set = std::make_shared<MapSet>(cv::Rect(0, 0, 1000, 1000), items);
    }
    ~Map() = default;

public:
    cv::Mat view(const MapSprite &sprite)
    {
        return {};
    }

public:
    std::shared_ptr<MapSet> set;
    std::shared_ptr<MapResource> resource;
};