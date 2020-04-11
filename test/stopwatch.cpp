#include <catch.hpp>
#include <iostream>
#include <fmt/format.h>
#include <angel/utils/stopwatch.hpp>

TEST_CASE("test stopwatch" , "[stopwatch]")
{
    angel::stopwatch<>::duration_type time(0);
    {
        angel::stopwatch t(time);

    }

    std::cout<<fmt::format("{:5.2f} seconds passed\n", angel::to_seconds(time));
}