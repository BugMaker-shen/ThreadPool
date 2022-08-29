修改src/main.cpp里MyTask的run函数，线程池执行的就是这个run函数，编译后会在bin目录下生成可执行文件threadpool_v1

可通过setMode方法设置线程池的工作模式，包括FIXED和CACHED

编译：
./autobuild.sh

测试：
./test.sh
