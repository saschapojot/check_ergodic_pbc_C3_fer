cmake_minimum_required(VERSION 3.28)
project(pbc_C3_fer LANGUAGES CXX)

# Allow compiler override via environment variables

set(CMAKE_C_COMPILER "/opt/intel/oneapi/compiler/2025.1/bin/icx")


set(CMAKE_CXX_COMPILER "/opt/intel/oneapi/compiler/2025.1/bin/icpx")
set(AVX2_FLAGS "-xCORE-AVX2 -qopt-zmm-usage=high")

message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find MKL
set(MKL_ROOT "/opt/intel/oneapi/mkl/latest")
set(MKL_INCLUDE_DIR "${MKL_ROOT}/include")
set(MKL_LIBRARY_DIR "${MKL_ROOT}/lib/intel64")

find_library(MKL_RT_LIB mkl_rt HINTS ${MKL_LIBRARY_DIR})
if(NOT MKL_RT_LIB)
    message(FATAL_ERROR "MKL runtime library not found.")
endif()

message(STATUS "Found MKL runtime library: ${MKL_RT_LIB}")
message(STATUS "Using MKL include directory: ${MKL_INCLUDE_DIR}")

# Find Python
find_package(Python3 REQUIRED COMPONENTS Development)

# Find Boost
set(Boost_NO_SYSTEM_PATHS ON)
set(BOOST_ROOT "$ENV{HOME}/cppLocal/boostBuild")
find_package(Boost REQUIRED COMPONENTS python numpy filesystem)

# Armadillo configuration
find_path(ARMADILLO_INCLUDE_DIR armadillo HINTS "/home/adada/cppLocal/armadillo_intel/include")
find_library(ARMADILLO_LIBRARY armadillo HINTS "/home/adada/cppLocal/armadillo_intel/lib")

# Add executable
add_executable(run_mc
        run_mc.cpp
        mc_subroutine/mc_read_load_compute.cpp
)

# Compile options
target_compile_options(run_mc PRIVATE
        $<$<CONFIG:Release>:${AVX2_FLAGS} -O3 -mkl -qopenmp -march=native -ftree-vectorize>
        $<$<CONFIG:Debug>:-g -O0>
)

# Set specific optimization for critical files
set_source_files_properties(
        mc_subroutine/mc_read_load_compute.cpp
        run_mc.cpp
        PROPERTIES COMPILE_OPTIONS "-O3"
)

# Include directories
target_include_directories(run_mc PRIVATE
        ${MKL_INCLUDE_DIR}
        ${ARMADILLO_INCLUDE_DIR}
        ${Python3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(run_mc PRIVATE
        ${MKL_RT_LIB}
        pthread
        m
        ${ARMADILLO_LIBRARY}
        Boost::filesystem
        Boost::python
        Boost::numpy
        Python3::Python
)

# Add common flags
target_compile_options(run_mc PRIVATE -qopenmp)
target_link_options(run_mc PRIVATE -qopenmp)