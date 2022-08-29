#include "public.hpp"

Task::Task()
	:resultPtr_(nullptr)
{}

Task::~Task() = default;

void Task::exec() {
	if (resultPtr_ != nullptr) {
		// Task派生类对象调用run方法，返回的Any类型封装了数据，设置到Task执行完后对应的Result里面，这个resultPtr_指向的对象是submitTask返回时构造的
		// run返回Any类型，Result对象底层有一个Any成员
		resultPtr_->setVal(run());   // 发生多态，调用的是用户传入的派生任务类的run，而不是虚基类Task的run
	}
}

void Task::setResult(Result* resultPtr) {
	resultPtr_ = resultPtr;
}


Result::Result(std::shared_ptr<Task> taskPtr, bool isSubmitSucceed)
	: isSubmitSucceed_(isSubmitSucceed)
{
	// 指定一个Result对象，task执行完就把结果存入这个Result对象
	taskPtr->setResult(this);
}

Result::~Result() = default;

// setVal是线程池执行完用户任务对象的run方法后，调用的
void Result::setVal(Any any) {
	// 从task拿到返回值，用户调用的get就可以返回了
	this->any_ = std::move(any);
	sem_.post();
}

// get是用户调用的
Any Result::get() {
	if (!isSubmitSucceed_) {
		return NULL_RESULT;
	}
	// 任务如果没有执行完，这里会阻塞
	sem_.wait();
	return std::move(any_);  // move后any_就没有资源了
}
