project(gen_map)


set(OpenCV_DIR "${ThridPatryDir}/opencv-xfeatures2d-release-static/x64/vc17/staticlib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp) 
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# set msvc runtime  as multi-threaded
set_target_properties(${PROJECT_NAME}  PROPERTIES COMPILE_FLAGS "/MT")

# copy map/*.png to exe folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/map/"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/map")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/overlay/"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/overlay")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/city/"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/city")
