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

# copy sricpt *.exe to bin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/sricpt/bzip2.exe
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
