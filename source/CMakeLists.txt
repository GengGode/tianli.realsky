set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(src_utils 
tianli.utils/utils.operation.image.h
tianli.utils/utils.convect.image.h
tianli.utils/utils.convect.string.h
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
#tianli.ui/form/tianli.map.h
#tianli.ui/form/tianli.map.cpp
#tianli.ui/form/tianli.map.ui
tianli.ui/form/mapview.h
tianli.ui/form/mapview.cpp
tianli.ui/form/mapview.ui
)

# include ui_*.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# set msvc runtime  as multi-threaded
set_target_properties(${PROJECT_NAME}  PROPERTIES COMPILE_FLAGS "/MT")

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
set(CMAKE_BUILD_TYPE "Debug")
#set(Qt5VER "qt-5.15.2-static-release-only")
set(Qt5VER "qt-5.15.2-lite-release-static")
# qt5
set(Qt5_DIR "${ThridPatryDir}/${Qt5VER}/lib/cmake/Qt5")
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME}  Qt5::Core Qt5::Gui Qt5::Widgets)
# qt5 plugins qwindows


set(OpenCV_DIR "${ThridPatryDir}/opencv-xfeatures2d-release-static/x64/vc17/staticlib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

#add_subdirectory(tianli.ui.resources)

