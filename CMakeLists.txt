cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0037 NEW)
PROJECT(OCVvid2fulldome)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/")

#find_package(OpenCV REQUIRED)

#include_directories($(OpenCV_INCLUDE_DIR))

aux_source_directory(. SRC_LIST)

add_executable(OCVvid2fulldome OCVvid2fulldome.cpp)
#target_link_libraries(OCVvid2fulldome ${OpenCV_LIBS}  )

