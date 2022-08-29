#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "thread.hpp"
#include "public.hpp"

#include <unordered_map>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

enum class PoolMode {
	MODE_FIXED,   // 线程数量固定
	MODE_CACHED,  // 线程数量动态增长
};

// 线程池类型
class ThreadPool {
public:
	// 构造析构一定要成对出现，这时编程规范
	ThreadPool(PoolMode poolMode = PoolMode::MODE_FIXED);
	~ThreadPool();

	// 设置线程池的工作方式
	void setMode(PoolMode poolMode);

	// 设置线程池初始的线程数量
	void setInitThreadSize(size_t initThreadSize);

	void setThreadThreshHold(size_t threshHold);

	// 设置任务队列上限
	void setTaskQueueMaxThreshHold(size_t threshHold);

	// 给线程池提交任务
	const Result submitTask(std::shared_ptr<Task> taskPtr);

	// 启动线程池
	void start(size_t initThreadSize = std::thread::hardware_concurrency());

	// 防止用户对线程池拷贝构造和赋值
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// 线程函数
	void threadFunc(unsigned int threadId);

	// 获取线程池运行状态 只读不写的方法，实现成const方法
	bool inline getPoolState() const;

private:
	PoolMode poolMode_;               // 线程池工作方式
	std::atomic_bool isPoolRunning_;  // 当前线程池的启动状态

	// std::vector<std::unique_ptr<Thread>> threads_;  
	std::unordered_map<unsigned int, std::unique_ptr<Thread>> threads_;   // Thread对象用不带引用计数的unique_ptr指向，ThreadPool对象出作用域，相应的成员变量也会出作用域，unique_ptr自动析构
	
	size_t initThreadSize_;                         // 初始线程数量
	std::atomic_uint freeThreadSize_;               // 空闲线程的数量
	// std::atomic_uint poolThreadSize_;               // 记录线程池中的线程数量
	size_t threadSizeMaxThreshHold_;                // cached模式下，线程数量阈值

	// 用于线程互斥访问taskQueue_
	std::mutex taskQueueMtx_;
	// 如果用户传入一个临时对象，我们使用裸指针接收，临时对象析构后这个裸指针就成了野指针了
	// 使用Task拷贝构造，在queue里构造一个对象也不行，我们希望用基类指针指向派生类对象，就能调用派生类对象的run方法，此外Task是虚基类，无法实例化对象
	// 防止用户传入的只是一个临时对象，使用shared_ptr保持对象的生命周期，等线程执行完run方法才能析构任务对象
	std::queue<std::shared_ptr<Task>> taskQueue_;
	size_t taskSizeMaxThreshHold_;  // 任务队列上限
	// 记录任务数量的变量，外部线程提交任务就++，线程池的线程执行任务就--，不同的线程都会访问这个变量，需要线程安全，为什么不使用taskQueue_.size()？
	// std::atomic_uint undoTaskSize_;
	

	// 对于池对象，通常定义两个条件变量notFull_和notEmpty_，表示任务队列状态
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;

	// ThreadPool析构时使用
	std::condition_variable exitCond_;
};


#endif