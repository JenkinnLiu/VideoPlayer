#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include <Windows.h>
#include <varargs.h>
class ETool {
public:
	static void ETrace(const char* format, ...) {//输出调试信息
		va_list ap; // 定义一个va_list类型的变量，用来存储可变参数的列表
		va_start(ap, format); // 初始化ap，让它指向第一个可变参数
		std::string sBuffer; // 定义一个字符串缓冲区
		sBuffer.resize(1024 * 10); // 为缓冲区分配内存
		vsprintf((char*)(sBuffer.c_str()), format, ap); // 将可变参数格式化输出到字符串缓冲区
		OutputDebugStringA(sBuffer.c_str()); // 输出调试信息
		va_end(ap); // 将ap置为NULL
	}
};

#ifndef TRACE
#define TRACE ETool::ETrace
#endif

class ThreadFuncBase {};
typedef int (ThreadFuncBase::* FUNCTYPE)();
class ThreadWorker {
public:
	ThreadWorker() :thiz(NULL), func(NULL) {};

	ThreadWorker(void* obj, FUNCTYPE f) :thiz((ThreadFuncBase*)obj), func(f) {} //构造函数

	ThreadWorker(const ThreadWorker& worker) { //拷贝构造函数
		thiz = worker.thiz;
		func = worker.func;
	}
	ThreadWorker& operator=(const ThreadWorker& worker) { //重载赋值运算符
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}

	int operator()() { //重载函数调用运算符
		if (IsValid()) {
			return (thiz->*func)();
		}
		return -1;
	}
	bool IsValid() const {
		return (thiz != NULL) && (func != NULL);
	}
private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};


class EdoyunThread //线程类
{
public:
	EdoyunThread() {
		m_hThread = NULL;
		m_bStatus = false;
	}

	~EdoyunThread() {
		Stop();
	}

	//true 表示成功 false表示失败
	bool Start() { // 启动线程
		m_bStatus = true; // 设置线程状态为运行
		m_hThread = (HANDLE)_beginthread(&EdoyunThread::ThreadEntry, 0, this); // 创建线程，调用ThreadEntry函数
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus; // 返回线程状态
	}

	bool IsValid() {//返回true表示有效 返回false表示线程异常或者已经终止
		if (m_hThread == NULL || (m_hThread == INVALID_HANDLE_VALUE))return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT; // 检查线程是否已经终止
	}

	bool Stop() { // 停止线程
		if (m_bStatus == false)return true;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, 1000); // 等待线程终止
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread, -1); // 强制终止线程
		}
		UpdateWorker(); // 更新工作线程
		return ret == WAIT_OBJECT_0;
	}

	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker()) { // 更新工作线程
		if (m_worker.load() != NULL && (m_worker.load() != &worker)) { // 如果工作线程不为空，且不等于worker
			::ThreadWorker* pWorker = m_worker.load(); // 获取工作线程																	
			TRACE("delete pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load()); // 输出调试信息
			m_worker.store(NULL); // 清空工作线程
			delete pWorker;
		}
		if (m_worker.load() == &worker)return; // 如果工作线程等于worker，返回
		if (!worker.IsValid()) { // 如果worker无效
			m_worker.store(NULL); // 清空工作线程
			return;
		}
		::ThreadWorker* pWorker = new ::ThreadWorker(worker);
		TRACE("new pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
		m_worker.store(pWorker); // 设置工作线程
	}

	//true表示空闲 false表示已经分配了工作
	bool IsIdle() { // 判断线程是否空闲
		if (m_worker.load() == NULL)return true;
		return !m_worker.load()->IsValid();
	}
private:
	void ThreadWorker() { // 线程工作函数，不断检查工作线程，设为私有函数是因为只能在类内部调用
		while (m_bStatus) {
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();
					if (ret != 0) {
						TRACE("thread found warning code %d\r\n", ret);
					}
					if (ret < 0) {
						::ThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}
			}
			else {
				Sleep(1);
			}
		}
	}
	static void ThreadEntry(void* arg) { // 线程入口函数
		EdoyunThread* thiz = (EdoyunThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread; // 线程句柄
	bool m_bStatus;//false 表示线程将要关闭  true 表示线程正在运行
	std::atomic<::ThreadWorker*> m_worker; //工作线程,原子操作
};

class EdoyunThreadPool //线程池类
{
public:
	EdoyunThreadPool(size_t size) { // 构造函数
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
			m_threads[i] = new EdoyunThread(); // 创建线程
	}
	EdoyunThreadPool() {}
	~EdoyunThreadPool() {
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++) {
			delete m_threads[i]; // 删除线程
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}
	bool Invoke() { // 启动线程池
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) { // 遍历线程
			if (m_threads[i]->Start() == false) { // 启动线程
				ret = false;
				break;
			}
		}
		if (ret == false) {
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop(); // 停止线程
			}
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}

	//返回-1 表示分配失败，所有线程都在忙 大于等于0，表示第n个线程分配来做这个事情
	int DispatchWorker(const ThreadWorker& worker) { // 分配工作线程
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) { // 如果线程空闲
				m_threads[i]->UpdateWorker(worker); // 更新工作线程
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) { // 检查线程是否有效
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<EdoyunThread*> m_threads;
};