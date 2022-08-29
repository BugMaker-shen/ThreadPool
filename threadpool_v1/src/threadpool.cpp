#include "threadpool.hpp"
#include "thread.hpp"

#include <iostream>
#include <functional>

const size_t TASK_SIZE_MAX_THRESHHOLH = INT32_MAX;
const size_t THREAD_SIZE_MAX_THRESHHOLH = 100U;
const size_t THREAD_MAX_FREE_SECONDS = 10U;


// 用户传入参数就用参数初始化，不传入参数就默认初始化
ThreadPool::ThreadPool(PoolMode poolMode)
	: poolMode_(poolMode)
	, isPoolRunning_(false)
	, taskSizeMaxThreshHold_(TASK_SIZE_MAX_THRESHHOLH)
	, initThreadSize_(0)
	, freeThreadSize_(0)
	// , poolThreadSize_(0)
	, threadSizeMaxThreshHold_(THREAD_SIZE_MAX_THRESHHOLH)
{}

// 由于没有使用堆区空间，析构函数不用释放
ThreadPool::~ThreadPool() {
	// 析构的时候，线程池中的线程可能在notEmpty_上等待任务到来，条件成立，也可能正在执行任务
	isPoolRunning_ = false;

	// 确保现在析构的时候没有线程在访问队列，所以也要抢到这个队列的互斥锁
	std::unique_lock<std::mutex> lock(taskQueueMtx_);
	

	// 线程池要退出了，通知在notEmpty_上等待任务的线程起来
	notEmpty_.notify_all();

	// 需要等到线程池中的线程全部结束，线程容器中没有元素后，退出条件成立
	std::cout << "主线程开始wait" << std::endl;
	exitCond_.wait(lock, [&]() {return threads_.size() == 0 && taskQueue_.size() == 0; });
	std::cout << "主线程wait完成" << std::endl;
}

// 设置线程池的工作方式
void ThreadPool::setMode(PoolMode poolMode) {
	if (getPoolState()) {
		std::cout << "Thread Pool is running, can not set mode" << std::endl;
		return;
	}
	poolMode_ = poolMode;
}

// 设置线程池初始的线程数量
void ThreadPool::setInitThreadSize(size_t initThreadSize) {
	if (getPoolState()) {
		std::cout << "Thread Pool is running, can not set init thread size" << std::endl;
		return;
	}
	initThreadSize_ = initThreadSize;
}

// 设置cached模式下，创建线程数量上限
void ThreadPool::setThreadThreshHold(size_t threshHold) {
	if (getPoolState() || poolMode_ == PoolMode::MODE_FIXED) {
		// 线程池停止或fixed，不允许设置线程数量上限
		std::cout << "Thread Pool is running or FIXED, can not set thread max threshhold" << std::endl;
		return;
	}
	threadSizeMaxThreshHold_ = threshHold;
}


// 设置任务队列上限
void ThreadPool::setTaskQueueMaxThreshHold(size_t threshHold) {
	if (getPoolState()) {
		std::cout << "Thread Pool is running or FIXED, can not set task queue max threshhold" << std::endl;
		return;
	}
	taskSizeMaxThreshHold_ = threshHold;
}

// 给线程池提交任务，在任务队列中插入任务，相当于生产者线程
const Result ThreadPool::submitTask(std::shared_ptr<Task> taskPtr) {
	// 获取锁
	std::unique_lock<std::mutex> lock(taskQueueMtx_);
	// 线程通信 等待队列不满 通过condition_variable 
	// 不能一直阻塞用户的线程，如果条件不满足，用户线程阻塞，1s内若还是无法将任务放入任务队列，则返回任务提交失败，保证服务能正常运行
	
	// 条件不成立，释放锁，等待直到条件成立，重新获取锁
	// notFull_见名知意，如果不阻塞，则表示条件成立，即不满
	// 如果提交时间超过1s，不能阻塞用户线程太长时间，告知用户提交失败
	bool isNotFull = notFull_.wait_for(lock, std::chrono::seconds(1), 
		[&]()->bool { return taskQueue_.size() < taskSizeMaxThreshHold_; });

	if (!isNotFull) {
		// 任务队列满了
		std::cerr << "task queue is full, submit task fail!" << std::endl;
		return { taskPtr, false };
	}

	// 如果空余，把任务放入任务队列
	taskQueue_.emplace(taskPtr);
	// 原子类型，记录任务队列中元素数量
	// undoTaskSize_++;

	// 因为放入了任务，需要通知等待在notEmpty_条件变量上的线程消费
	notEmpty_.notify_all();

	// 根据任务数量和空闲线程的数量，判断是否需要创建新的线程
	if (poolMode_ == PoolMode::MODE_CACHED && freeThreadSize_ < taskQueue_.size() && threads_.size() < threadSizeMaxThreshHold_) {
		// cached && 没处理的任务数量大于空闲线程数量 && 线程数没到阈值 -> 创建新的线程
		/*std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		threads_.emplace_back(std::move(ptr));*/

		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		unsigned int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		// FIXED模式下，用户在线程池start时候启动线程，CACHED模式下，用户在提交任务后需要start线程
		threads_[threadId]->start();
		// 记录线程池中线程数量
		// poolThreadSize_++;
		// 记录线程池中空闲线程数量
		freeThreadSize_++;
		std::cout << " a new thread created!" << std::endl;
	}

	// 返回Result对象  Result用shared_ptr指向task对象，Result在，task对象就在
	return { taskPtr };
}

bool inline ThreadPool::getPoolState() const {
	return isPoolRunning_;
}

// 启动线程池
void ThreadPool::start(size_t initThreadSize) {
	isPoolRunning_ = true;
	// 调用start方法需要设置初始的线程数量
	initThreadSize_ = initThreadSize;
	
	// 创建initThreadSize_个线程对象
	for (size_t i = 0; i < initThreadSize_; i++) {
		// 创建Thread对象的时候，把线程函数ThreadPool::threadFunc给Thread对象，后面Thread对象调用start方法就不用传入ThreadPool对象了
		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		// move做资源转移，将左值转换为右值，直接把实参的资源转移到了vector的元素中
		// 用户线程插入到threads_中时，thread对象还没有启动，所以不能使用OS给线程分配的id作为key
		threads_.emplace(ptr->getId(), std::move(ptr));
	}

	// 遍历map，启动所有线程
	for (auto iter = threads_.begin(); iter != threads_.end(); iter++) {
		iter->second->start();
	}
	//for (size_t i = 0; i < initThreadSize_; i++) {
	//	threads_[i]->start();    // 线程启动，需要执行一个线程函数，即ThreadPool::taskQueue中的任务
	//}
	freeThreadSize_ = initThreadSize_;  // 刚创建线程时，空闲线程等于初始线程数量
}

// 定义线程启动的线程函数，线程池中的所有线程都执行线程池锁规定的线程函数
// 线程函数结束，线程也就结束了
void ThreadPool::threadFunc(unsigned int threadId) {
	// 开始空闲的时间有两个，开始执行线程函数、任务执行完
	auto freeStartTime = std::chrono::high_resolution_clock().now();

	// 线程池启动后，会创建并启动线程，执行线程方法，这个方法执行起来，就进入了死循环，会不断执行线程方法，没有任务时就会阻塞在notEmpty_条件变量上
	while (true) {
		std::shared_ptr<Task> taskPtr;
		{
			// 获取锁
			std::unique_lock<std::mutex> lock(taskQueueMtx_);

			// cached模式下，需要回收空闲时间过长的线程
			while (taskQueue_.size() == 0) {
				if (!getPoolState()) {
					// 优先判断是否有任务：没任务了 && 线程池停止 ==> 才退出      
					// 线程池结束 && 任务执行完才能结束
					threads_.erase(threadId);
					// 记录线程数量的相关变量值修改
					freeThreadSize_--;
					std::cout << "pool exit, tid = " << std::this_thread::get_id() << " exit! threads_.size() = " << threads_.size() << std::endl;
					exitCond_.notify_all();
					return;
					// 所有线程统一从这里退出
				}
				// 没有任务，才能进入while，等待在wait
				if (poolMode_ == PoolMode::MODE_CACHED) {
					// 如果没任务了，线程需要1s返回一次，查看是否达到空闲上限，这里的wait会释放锁
					if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1))) {
						// 等待在notEmpty_，超时，表示一直没有任务到来
						auto now = std::chrono::high_resolution_clock().now();
						auto span = std::chrono::duration_cast<std::chrono::seconds>(now - freeStartTime);
						if (span.count() > THREAD_MAX_FREE_SECONDS && threads_.size() > initThreadSize_) {
							// 回收线程
							// 把线程对象从线程容器中删除    执行当前线程函数的线程对应容器里哪个元素？
							threads_.erase(threadId);
							// 记录线程数量的相关变量值修改
							// poolThreadSize_--;
							freeThreadSize_--;
							std::cout << "tid = " << std::this_thread::get_id() << " is free more than " << THREAD_MAX_FREE_SECONDS << "s, exit" << " threads_.size() = " << threads_.size() << std::endl;
							
							return;
						}
					}
				}
				else {
					// FIXED模式
					// 执行当前方法的是线程池里的线程，等不到任务就会一直等待，直到notEmpty_条件成立
					notEmpty_.wait(lock);
				}
			}
			

			// 进入上面while再出来或者不进入while循环，必然是因为有任务或者线程池退出了，有任务才执行以下代码
			freeThreadSize_--;

			// 从任务队列中取一个任务
			taskPtr = taskQueue_.front();
			taskQueue_.pop();
			// undoTaskSize_--;

			std::cout << "tid = " << std::this_thread::get_id() << " get the task successfully !" << std::endl;

			// 从任务队列取出任务后，依然不空，通知其他阻塞在notEmpty_上的线程可以消费任务
			if (taskQueue_.size() > 0) {
				notEmpty_.notify_all();
			}

			// 获取一个任务后，notFull_就成立了，可以submitTask了
			notFull_.notify_all();
		}
		// 出了作用域，用户互斥访问任务队列的锁就析构了
		std::cout << "tid = " << std::this_thread::get_id() << " free task Queue Mutex" << std::endl;

		// 当前线程负责执行这个任务
		if (taskPtr != nullptr) {
			std::cout << "tid = " << std::this_thread::get_id() << " start run the task..." << std::endl;
			taskPtr->exec();
			std::cout << "tid = " << std::this_thread::get_id() << " finish the task !" << std::endl;
		}
		freeThreadSize_++;
		// 线程执行完任务的时间
		freeStartTime = std::chrono::high_resolution_clock().now();
	}
}
