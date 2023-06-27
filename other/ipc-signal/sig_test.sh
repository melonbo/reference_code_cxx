#!/bin/bash

# 定义信号处理函数
handle_signal() {
    echo "Received signal $1"
}

# 注册信号处理函数
trap 'handle_signal SIGINTs' 31
trap 'handle_signal SIGTERM' SIGTERM

echo "Running..."

# 模拟长时间运行的任务
sleep 100

echo "Finished."

