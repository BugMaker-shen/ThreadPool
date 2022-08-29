#ifndef THREAD_H
#define THREAD_H

#include <functional>
#include <thread>

// 线程类型
class Thread {
public:
	using ThreadFunc = std::function<void(unsigned int)>;

	Thread(ThreadFunc func);
	~Thread();

	// 获取线程id
	unsigned int getId() const;

	// 启动线程
	void start();
private:
	// 线程函数对象，由线程池创建线程对象时指定
	ThreadFunc func_;
	static unsigned int generateId_;
	unsigned int threadId_;
};

#endif