#ifndef ANY_H
#define ANY_H

#include <memory>
#include <iostream>

class Any {
public:
	// 由于唯一的成员base_是unique_ptr删除了左值拷贝构造和赋值，所以Any最好也删除拷贝构造和赋值
	Any() = default;
	~Any() = default;

	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;

	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	// 这个构造函数可以让Any类型封装任意类型的数据
	template<typename T>
	Any(T data)
		: base_(std::make_unique<Derive<T>>(data))
	{}

	// 这个方法返回Any类型封装的数据
	template<typename T>
	T cast_() {
		// 派生类访问基类成员，继承后可以直接通过作用域访问
		// 基类指针指向派生类对象，需要范围派生类成员时，得转换为派生类指针
		// 需要用支持RTTI类型转换的dynamic_cast，将Base*转为Derive*
		// 如果说计算的是float，那base_指向Derive<float>，如果调用cast_给的模板参数是int，则需要用dynamic_cast将Derive<float>转为Derive<int>，转换失败，返回nullptr
		Derive<T>* d_ptr = dynamic_cast<Derive<T>*>(base_.get());
		if (d_ptr == nullptr) {
			throw "type is incompatible!";
		}
		// 只是做了类型转换，相当于两个指针指向同一个资源
		return d_ptr->data_;
	}

private:
	class Base {
	public:
		// 虚析构函数，用Base*指向Derive对象时，delete *base_ptr也能执行~Derive()
		virtual ~Base() = default;
	};

	template<typename T>
	class Derive : public Base {
	public:
		Derive(T data) :data_(data) {}
		T data_;
	};

private:
	std::unique_ptr<Base> base_;
};

#endif