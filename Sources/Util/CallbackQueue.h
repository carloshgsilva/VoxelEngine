#pragma once

#include <mutex>
#include <queue>
#include <functional>

//An Thread Safe Callback Queue
template<class T>
class CallbackQueue {
	std::mutex _Mutex;
	std::vector<std::function<void(T)>> _Callbacks;

public:
	void AddCallback(std::function<void(T)> _Func) {
		std::unique_lock l(_Mutex);
		_Callbacks.push_back(_Func);
	}

	void ExecuteAll(T _Data) {
		std::unique_lock l(_Mutex);
		for (auto& _Cb : _Callbacks) {
			_Cb(_Data);
		}
	}

	auto begin() { return _Callbacks.begin(); }
	auto end() { return _Callbacks.begin(); }
};

template<>
class CallbackQueue<void> {
	std::mutex _Mutex;
	std::vector<std::function<void()>> _Callbacks;

public:
	void AddCallback(std::function<void()> _Func) {
		std::unique_lock l(_Mutex);
		_Callbacks.push_back(_Func);
	}

	void ExecuteAll() {
		std::unique_lock l(_Mutex);
		for (auto& _Cb : _Callbacks) {
			_Cb();
		}
	}

	void ExecuteAndClear() {
		ExecuteAll();
		_Callbacks.clear();
	}

	auto begin() { return _Callbacks.begin(); }
	auto end() { return _Callbacks.begin(); }
};