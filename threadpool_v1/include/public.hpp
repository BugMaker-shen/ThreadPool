#ifndef PUBLIC_H
#define PUBLIC_H

#include <atomic>

#include "any.hpp"
#include "semaphore.hpp"

const int NULL_RESULT = 0;

class Result;

// 用户可以继承Task抽象基类，重写run方法，自定义任意任务类型
// 每个task指定返回数据的类型
class Task {
public:
	Task();

	~Task();

	void exec();

	void setResult(Result* resultPtr);

	virtual const Any run() = 0;

private:
	Result* resultPtr_;
};

// 用于接收submitTask的返回值类型，也需要指定返回类型
class Result {
public:
	Result(std::shared_ptr<Task> taskPtr, bool isSubmitSucceed = true);

	~Result();

	Result(const Result&) = delete;
	Result& operator=(const Result&) = delete;

	Result(Result&&) = default;
	Result& operator=(Result&&) = default;

	// setVal是线程池执行完用户任务对象的run方法后，调用的
	void setVal(Any any);

	// get是用户调用的
	Any get();

private:
	Any any_;
	Semaphore sem_;
	// Task* taskPtr_;    // 使用裸指针，使用shared_ptr，Task和Result对象存在交叉引用问题
	std::atomic_bool isSubmitSucceed_;
};

#endif