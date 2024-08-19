cmake_minimum_required(VERSION 2.8)
project(main)

# 配置编译选项
# -g 可调试
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 指定搜索的子目录
add_subdirectory(testmuduo)