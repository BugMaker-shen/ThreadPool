# 要求cmake程序的最低版本
cmake_minimum_required(VERSION 3.0)
# 工程名
project(threadpool)

# 配置编译选项，-g表示调试
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
set(CMAKE_CXX_STANDARD 17)


# 配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 配置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 加载子目录，在src下找CMakeLists.txt执行
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
