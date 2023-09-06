#include "combiner_inface.h"
#include "test.h"

class combiner_instance : public combiner_inface
{

    void init(std::vector<size> &objs) override
    {
    }
    std::map<int, rect> pre_combinered(size limit_size) override
    {
        return {};
    }
};

void test()
{
    std::vector<size> test_objs = objs;
    std::sort(test_objs.begin(), test_objs.end(), [](const size &a, const size &b)
              { return a.width * a.height > b.width * b.height; });

    size limit = {2048 * 3, 2048 * 5};

    combiner_inface *combiner = new combiner_instance();
    combiner->init(test_objs);
    auto result = combiner->pre_combinered(limit);
}

int main()
{
    test();
    return 0;
}