# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS /Users/patrice/esp/esp-idf-lib/components /Users/patrice/esp/mea-esp32-homekit-toolbox/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mea-osmolator)
