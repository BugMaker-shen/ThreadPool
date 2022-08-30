#!/bin/bash

set -e

# 如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

# 回到项目根目录
cd ..

if [ ! -d /usr/local/include/threadpool_v1 ]; then 
    mkdir /usr/local/include/threadpool_v1
fi

# 把头文件拷贝到 /usr/include/threadpool_v1
for header in `ls include/*.hpp`
do
    cp $header /usr/local/include/threadpool_v1
done

# so库拷贝到 /usr/lib    PATH
cp `pwd`/lib/libthreadpool_v1.so /usr/local/lib

# 如果已经将so库拷贝到了PATH，编译器依然找不到库，需要执行一下ldconfig
ldconfig