cmake_minimum_required(VERSION 3.15)
project(get_item_json)

add_executable(get_item_json main.cpp)

# cpr
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) 
FetchContent_MakeAvailable(cpr)
# spdlog
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git
                            GIT_TAG v1.12.0) 
FetchContent_MakeAvailable(spdlog)

# cpr
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
# spdlog
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
