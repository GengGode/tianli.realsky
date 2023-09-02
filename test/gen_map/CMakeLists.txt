cmake_minimum_required(VERSION 3.10)
project(gen_map)


set(OpenCV_DIR "${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/lib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp  BlockMapResource.h) 
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# copy dll to exe folder
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ".${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/bin/opencv_world480d.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
else()
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/bin/opencv_world480d.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()