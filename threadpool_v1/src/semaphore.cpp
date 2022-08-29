#include "semaphore.hpp"

Semaphore::Semaphore(int srcLimit)
	:srcLimit_(srcLimit)
{}

Semaphore::~Semaphore() = default;

// P
void Semaphore::wait() {
	std::unique_lock<std::mutex> lock(mtx_);
	// 等待信号量有资源，无资源需要阻塞
	cv_.wait(lock, [&]()->bool {return srcLimit_ > 0; });
	srcLimit_--;
}

// V
void Semaphore::post() {
	std::unique_lock<std::mutex> lock(mtx_);
	srcLimit_++;
	cv_.notify_all();
}
