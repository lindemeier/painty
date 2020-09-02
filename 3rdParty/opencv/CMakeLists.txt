cmake_minimum_required(VERSION 3.14...3.18)

project(opencv LANGUAGES CXX)

include(FetchContent)

# FetchContent_Declare(
#   opencv_contrib
#   GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
#   GIT_TAG 4.4.0
# )
# FetchContent_MakeAvailable(opencv_contrib)

# set(OPENCV_EXTRA_MODULES_PATH ${opencv_contrib_SOURCE_DIR}/modules)
set(BUILD_TESTS OFF)
set(BUILD_JAVA OFF)
set(BUILD_SHARED_LIBS ON)

# Include opencv_alphamat module into the OpenCV build
set(BUILD_opencv_alphamat OFF)

# Build utility applications (used for example to train classifiers)
set(BUILD_opencv_apps OFF)

# Include opencv_aruco module into the OpenCV build
set(BUILD_opencv_aruco OFF)

# Include opencv_bgsegm module into the OpenCV build
set(BUILD_opencv_bgsegm OFF)

# Include opencv_bioinspired module into the OpenCV build
set(BUILD_opencv_bioinspired OFF)

# Include opencv_calib3d module into the OpenCV build
set(BUILD_opencv_calib3d ON)

# Include opencv_ccalib module into the OpenCV build
set(BUILD_opencv_ccalib OFF)

# Include opencv_core module into the OpenCV build
set(BUILD_opencv_core ON)

# Include opencv_datasets module into the OpenCV build
set(BUILD_opencv_datasets OFF)

# Include opencv_dnn module into the OpenCV build
set(BUILD_opencv_dnn OFF)

# Include opencv_dnn_objdetect module into the OpenCV build
set(BUILD_opencv_dnn_objdetect OFF)

# Include opencv_dnn_superres module into the OpenCV build
set(BUILD_opencv_dnn_superres OFF)

# Include opencv_dpm module into the OpenCV build
set(BUILD_opencv_dpm OFF)

# Include opencv_face module into the OpenCV build
set(BUILD_opencv_face OFF)

# Include opencv_features2d module into the OpenCV build
set(BUILD_opencv_features2d ON)

# Include opencv_flann module into the OpenCV build
set(BUILD_opencv_flann ON)

# Include opencv_freetype module into the OpenCV build
set(BUILD_opencv_freetype OFF)

# Include opencv_fuzzy module into the OpenCV build
set(BUILD_opencv_fuzzy OFF)

# Include opencv_gapi module into the OpenCV build
set(BUILD_opencv_gapi OFF)

# Include opencv_hdf module into the OpenCV build
set(BUILD_opencv_hdf OFF)

# Include opencv_hfs module into the OpenCV build
set(BUILD_opencv_hfs OFF)

# Include opencv_highgui module into the OpenCV build
set(BUILD_opencv_highgui ON)

# Include opencv_img_hash module into the OpenCV build
set(BUILD_opencv_img_hash OFF)

# Include opencv_imgcodecs module into the OpenCV build
set(BUILD_opencv_imgcodecs ON)

# Include opencv_imgproc module into the OpenCV build
set(BUILD_opencv_imgproc ON)

# Include opencv_intensity_transform module into the OpenCV build
set(BUILD_opencv_intensity_transform OFF)

# Include opencv_java module into the OpenCV build
set(BUILD_opencv_java OFF)

# Include opencv_java_bindings_generator module into the OpenCV
#  build
set(BUILD_opencv_java_bindings_generator OFF)

# Include opencv_line_descriptor module into the OpenCV build
set(BUILD_opencv_line_descriptor OFF)

# Include opencv_ml module into the OpenCV build
set(BUILD_opencv_ml OFF)

# Include opencv_objdetect module into the OpenCV build
set(BUILD_opencv_objdetect OFF)

# Include opencv_optflow module into the OpenCV build
set(BUILD_opencv_optflow OFF)

# Include opencv_phase_unwrapping module into the OpenCV build
set(BUILD_opencv_phase_unwrapping OFF)

# Include opencv_photo module into the OpenCV build
set(BUILD_opencv_photo OFF)

# Include opencv_plot module into the OpenCV build
set(BUILD_opencv_plot OFF)

# Include opencv_python2 module into the OpenCV build
set(BUILD_opencv_python2 OFF)

# Include opencv_python3 module into the OpenCV build
set(BUILD_opencv_python3 OFF)

# Include opencv_python_bindings_generator module into the OpenCV
#  build
set(BUILD_opencv_python_bindings_generator OFF)

# Include opencv_python_tests module into the OpenCV build
set(BUILD_opencv_python_tests OFF)

# Include opencv_quality module into the OpenCV build
set(BUILD_opencv_quality OFF)

# Include opencv_rapid module into the OpenCV build
set(BUILD_opencv_rapid OFF)

# Include opencv_reg module into the OpenCV build
set(BUILD_opencv_reg OFF)

# Include opencv_rgbd module into the OpenCV build
set(BUILD_opencv_rgbd OFF)

# Include opencv_saliency module into the OpenCV build
set(BUILD_opencv_saliency OFF)

# Include opencv_sfm module into the OpenCV build
set(BUILD_opencv_sfm OFF)

# Include opencv_shape module into the OpenCV build
set(BUILD_opencv_shape OFF)

# Include opencv_stereo module into the OpenCV build
set(BUILD_opencv_stereo OFF)

# Include opencv_stitching module into the OpenCV build
set(BUILD_opencv_stitching OFF)

# Include opencv_structured_light module into the OpenCV build
set(BUILD_opencv_structured_light OFF)

# Include opencv_superres module into the OpenCV build
set(BUILD_opencv_superres OFF)

# Include opencv_surface_matching module into the OpenCV build
set(BUILD_opencv_surface_matching OFF)

# Include opencv_text module into the OpenCV build
set(BUILD_opencv_text OFF)

# Include opencv_tracking module into the OpenCV build
set(BUILD_opencv_tracking OFF)

# Include opencv_ts module into the OpenCV build
set(BUILD_opencv_ts OFF)

# Include opencv_video module into the OpenCV build
set(BUILD_opencv_video ON)

# Include opencv_videoio module into the OpenCV build
set(BUILD_opencv_videoio ON)

# Include opencv_videostab module into the OpenCV build
set(BUILD_opencv_videostab OFF)

# Include opencv_viz module into the OpenCV build
set(BUILD_opencv_viz OFF)

# Include opencv_world module into the OpenCV build
set(BUILD_opencv_world OFF)

# Include opencv_xfeatures2d module into the OpenCV build
set(BUILD_opencv_xfeatures2d OFF)

# Include opencv_ximgproc module into the OpenCV build
set(BUILD_opencv_ximgproc OFF)

# Include opencv_xobjdetect module into the OpenCV build
set(BUILD_opencv_xobjdetect OFF)

# Include opencv_xphoto module into the OpenCV build
set(BUILD_opencv_xphoto OFF)

FetchContent_Declare(
  opencv
  GIT_REPOSITORY https://github.com/opencv/opencv.git
  GIT_TAG 4.4.0
)
FetchContent_MakeAvailable(opencv)

set(opencv_DIR ${FETCHCONTENT_BASE_DIR}/opencv-src)
set(opencv_contrib_DIR ${FETCHCONTENT_BASE_DIR}/opencv_contrib-src)

set(opencv_INCLUDE_DIRS "")

macro(add_opencv_module MODULE_NAME opencv_INCLUDE_DIRS)
  list(APPEND opencv_INCLUDE_DIRS ${opencv_DIR}/modules/${MODULE_NAME}/include)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    target_compile_options(opencv_${MODULE_NAME} PUBLIC -Wno-cast-align -Wno-float-equal -Wno-shift-sign-overflow -Wno-shadow-field -Wno-inconsistent-missing-destructor-override -Wno-weak-vtables -Wno-float-equal)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    target_compile_options(opencv_${MODULE_NAME} PUBLIC -Wno-cast-align -Wno-float-equal -Wno-shift-sign-overflow -Wno-shadow-field -Wno-inconsistent-missing-destructor-override -Wno-weak-vtables -Wno-float-equal)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
    target_compile_options(opencv_${MODULE_NAME} PUBLIC /W4 /WX)
  endif()
endmacro(add_opencv_module)

macro(add_opencv_contrib_module MODULE_NAME opencv_INCLUDE_DIRS)
  list(APPEND opencv_INCLUDE_DIRS ${opencv_contrib_DIR}/modules/${MODULE_NAME}/include)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    target_compile_options(opencv_${MODULE_NAME} PUBLIC -Wno-cast-align -Wno-float-equal -Wno-shift-sign-overflow -Wno-shadow-field -Wno-inconsistent-missing-destructor-override -Wno-weak-vtables -Wno-float-equal)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    target_compile_options(opencv_${MODULE_NAME} PUBLIC -Wno-cast-align -Wno-float-equal -Wno-shift-sign-overflow -Wno-shadow-field -Wno-inconsistent-missing-destructor-override -Wno-weak-vtables -Wno-float-equal)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
    target_compile_options(opencv_${MODULE_NAME} PUBLIC /W4 /WX)
  endif()
endmacro(add_opencv_contrib_module)

# add_opencv_module(calib3d, opencv_INCLUDE_DIRS)
list(APPEND opencv_INCLUDE_DIRS ${opencv_contrib_DIR}/modules/calib3d/include)
add_opencv_module(core opencv_INCLUDE_DIRS)
add_opencv_module(features2d opencv_INCLUDE_DIRS)
add_opencv_module(flann opencv_INCLUDE_DIRS)
add_opencv_module(highgui opencv_INCLUDE_DIRS)
add_opencv_module(imgcodecs opencv_INCLUDE_DIRS)
add_opencv_module(imgproc opencv_INCLUDE_DIRS)
# add_opencv_module(shape opencv_INCLUDE_DIRS)
add_opencv_module(video opencv_INCLUDE_DIRS)
add_opencv_module(videoio opencv_INCLUDE_DIRS)
# add_opencv_contrib_module(ximgproc opencv_INCLUDE_DIRS)

SET(opencv_INCLUDE_DIRS  "${opencv_INCLUDE_DIRS}" CACHE INTERNAL "opencv_INCLUDE_DIRS")
