####################################################
#   File Name:    autobuild.sh                     
#   Author:       ZHN                              
#   Email:        2906141298@qq.com                
#   Purpose：     一键编译脚本                     
#   Created Time: 2024年03月26日 星期二
####################################################
#!/bin/bash

# 用于启动脚本的调试模式
set -x

rm -rf "$(pwd)"/build/*

cd "$(pwd)"/build &&
    cmake .. &&
    make