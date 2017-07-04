#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#


cmake_minimum_required(VERSION 2.8.12)

PROJECT(IPPhone)

include(dlib/dlib/cmake)
include_directories(include)
include_directories(dlib/dlib/opencv)
set(IPPhone IPPhone.cpp dlib/dlib/opencv/cv_image.h dlib/dlib/opencv/to_open_cv.h ${OpenCV_INCLUDE_DIRS}/opencv.hpp)
set(server server.cpp dlib/dlib/opencv/cv_image.h dlib/dlib/opencv/to_open_cv.h ${OpenCV_INCLUDE_DIRS}/opencv.hpp)
set(client client.cpp dlib/dlib/opencv/cv_image.h dlib/dlib/opencv/to_open_cv.h ${OpenCV_INCLUDE_DIRS}/opencv.hpp)
set(CMAKE_CXX_STANDARD 11)


# OpenCV_FOUND
# set(${OpenCV_INCLUDE_DIRS} opencv/include)
# <NAME>_LIBRARIES or <NAME>_LIBRARIES or <NAME>_LIBS
# <NAME>_DEFINITIONS


if (DLIB_NO_GUI_SUPPORT)
   message("No GUI support, so we won't build the webcam_face_pose_ex example.")
else()
   find_package(OpenCV REQUIRED)
   if (OpenCV_FOUND)
      include_directories(${OpenCV_INCLUDE_DIRS})
        ADD_EXECUTABLE(IPPhone IPPhone)
        ADD_EXECUTABLE(server server)
        ADD_EXECUTABLE(client client)
        #   ADD_EXECUTABLE(webcam_face_pose_ex webcam_face_pose_ex.cpp)
        TARGET_LINK_LIBRARIES(IPPhone dlib::dlib  ${OpenCV_LIBS} -lstdc++)
        TARGET_LINK_LIBRARIES(server  dlib::dlib ${OpenCV_LIBS} pthread -lstdc++)
        TARGET_LINK_LIBRARIES(client dlib::dlib ${OpenCV_LIBS} pthread )
   else()
      message("OpenCV not found, so we won't build the webcam_face_pose_ex example.")
   endif()
endif()


# if (DLIB_LINK_WITH_SQLITE3)
#    add_example(sqlite_ex)
# endif()


