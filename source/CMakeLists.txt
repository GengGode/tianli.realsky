set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(src_utils 
tianli.utils/utils.image.h
tianli.utils/utils.string.h
)

add_executable(${PROJECT_NAME} 
main.cpp 
${src_utils}
tianli.init.h
tianli.core/tianli.map.h
tianli.ui/tianli.window.h 
tianli.ui/tianli.window.cpp 
tianli.ui/tianli.window.ui 
tianli.ui/tianli.window.qrc
tianli.ui/form/mapview.h
tianli.ui/form/mapview.cpp
tianli.ui/form/mapview.ui
)

# include ui_*.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# set msvc runtime  as multi-threaded
set_target_properties(${PROJECT_NAME}  PROPERTIES COMPILE_FLAGS "/MTd")

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
set(CMAKE_BUILD_TYPE "Debug")
#set(Qt5VER "qt-5.15.2-static-release-only")
set(Qt5VER "qt-5.15.2-lite-release-static")
# qt5
set(Qt5_DIR "${ThridPatryDir}/${Qt5VER}/lib/cmake/Qt5")
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME}  Qt5::Core Qt5::Gui Qt5::Widgets)
# qt5 plugins qwindows
set(QT_PLUGINS_DIR "${ThridPatryDir}/${Qt5VER}/plugins")
target_link_libraries(${PROJECT_NAME}  "${QT_PLUGINS_DIR}/platforms/qwindowsd.lib")
target_link_libraries(${PROJECT_NAME}  "${QT_PLUGINS_DIR}/styles/qwindowsvistastyled.lib")


set(OpenCV_DIR "${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/lib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# copy map/*.png to exe folder
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_CURRENT_SOURCE_DIR}/map/"
#        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/map")

# copy dll to exe folder
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/bin/opencv_world480d.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
else()
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ThridPatryDir}/opencv-xfeatures2d-shared-world/x64/vc17/bin/opencv_world480.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()

#add_subdirectory(tianli.ui.resources)

