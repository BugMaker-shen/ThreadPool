修改src/main.cpp里MyTask的run函数，线程池执行的就是这个run函数

可通过setMode方法设置线程池的工作模式，包括FIXED和CACHED

构建项目：./autobuild.sh

构建项目后，会在lib下生成libthreadpool_v1.so，然后将include下的头文件拷贝到/usr/local/include/threadpool_v1，将so库拷贝到/usr/local/lib下

编译命令：g++ -o main main.cpp -std=c++17 -lthreadpool_v1 -lpthread

测试：./test.sh

main函数中有使用模板