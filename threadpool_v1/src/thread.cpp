#include "thread.hpp"

unsigned int Thread::generateId_ = 0;

// 线程池创建线程对象时，指定线程函数
Thread::Thread(ThreadFunc func) 
	: func_(func)
	, threadId_(generateId_++)
{}


Thread::~Thread() 
{}

unsigned int Thread::getId() const {
	return threadId_;
}

// 需要执行一个线程函数，这个函数应该由外界传给线程池，线程池指定给Thread对象，然后执行这个函数
// 这个函数对象从线程池传给Thread对象，在Thread对象构造时，用bind给Thread成员赋值
void Thread::start() {
	// 创建一个线程执行线程函数
	std::thread t(func_, threadId_);
	t.detach();
}