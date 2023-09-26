# help: https://medium.com/@onur.dundar1/cmake-tutorial-585dd180109b
cmake_minimum_required(VERSION 3.20)
project(fibers VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# This is the full path to shared-libs/lib
# set(SHARED_LIBS_FOLDER /home/linux-dev/Documents/dev/flex-iot/full-system/FlexNext-System/shared-libs/lib)
# set(SHARED_HEADERS_FOLDER /home/linux-dev/Documents/dev/flex-iot/full-system/FlexNext-System/shared-libs/include)

include(FetchContent)
# set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 10.1.1
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(fmtlib)

FetchContent_Declare(tsl_robin_map
    GIT_REPOSITORY https://github.com/Tessil/robin-map.git
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(tsl_robin_map)

# external libs
# lua and sol2
# add_subdirectory(vendor/lua)

# lockfree data structures
FetchContent_Declare(lockfreelib
    GIT_REPOSITORY "https://github.com/DNedic/lockfree"
    GIT_TAG        "2.0.4"
)
# set any loguru compile-time flags before calling MakeAvailable()
FetchContent_MakeAvailable(lockfreelib)

# loguru (should move this to submodule, as there are changes to it)
FetchContent_Declare(logurulib
    GIT_REPOSITORY "https://github.com/emilk/loguru"
    GIT_TAG        "master"
)
# set any loguru compile-time flags before calling MakeAvailable()
set(LOGURU_USE_FMTLIB TRUE)
FetchContent_MakeAvailable(logurulib)

add_subdirectory(src)

