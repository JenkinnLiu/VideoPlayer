#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include <Windows.h>
#include <varargs.h>
class ETool {
public:
	static void ETrace(const char* format, ...) {//���������Ϣ
		va_list ap; // ����һ��va_list���͵ı����������洢�ɱ�������б�
		va_start(ap, format); // ��ʼ��ap������ָ���һ���ɱ����
		std::string sBuffer; // ����һ���ַ���������
		sBuffer.resize(1024 * 10); // Ϊ�����������ڴ�
		vsprintf((char*)(sBuffer.c_str()), format, ap); // ���ɱ������ʽ��������ַ���������
		OutputDebugStringA(sBuffer.c_str()); // ���������Ϣ
		va_end(ap); // ��ap��ΪNULL
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

	ThreadWorker(void* obj, FUNCTYPE f) :thiz((ThreadFuncBase*)obj), func(f) {} //���캯��

	ThreadWorker(const ThreadWorker& worker) { //�������캯��
		thiz = worker.thiz;
		func = worker.func;
	}
	ThreadWorker& operator=(const ThreadWorker& worker) { //���ظ�ֵ�����
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}

	int operator()() { //���غ������������
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


class EdoyunThread //�߳���
{
public:
	EdoyunThread() {
		m_hThread = NULL;
		m_bStatus = false;
	}

	~EdoyunThread() {
		Stop();
	}

	//true ��ʾ�ɹ� false��ʾʧ��
	bool Start() { // �����߳�
		m_bStatus = true; // �����߳�״̬Ϊ����
		m_hThread = (HANDLE)_beginthread(&EdoyunThread::ThreadEntry, 0, this); // �����̣߳�����ThreadEntry����
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus; // �����߳�״̬
	}

	bool IsValid() {//����true��ʾ��Ч ����false��ʾ�߳��쳣�����Ѿ���ֹ
		if (m_hThread == NULL || (m_hThread == INVALID_HANDLE_VALUE))return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT; // ����߳��Ƿ��Ѿ���ֹ
	}

	bool Stop() { // ֹͣ�߳�
		if (m_bStatus == false)return true;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, 1000); // �ȴ��߳���ֹ
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread, -1); // ǿ����ֹ�߳�
		}
		UpdateWorker(); // ���¹����߳�
		return ret == WAIT_OBJECT_0;
	}

	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker()) { // ���¹����߳�
		if (m_worker.load() != NULL && (m_worker.load() != &worker)) { // ��������̲߳�Ϊ�գ��Ҳ�����worker
			::ThreadWorker* pWorker = m_worker.load(); // ��ȡ�����߳�																	
			TRACE("delete pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load()); // ���������Ϣ
			m_worker.store(NULL); // ��չ����߳�
			delete pWorker;
		}
		if (m_worker.load() == &worker)return; // ��������̵߳���worker������
		if (!worker.IsValid()) { // ���worker��Ч
			m_worker.store(NULL); // ��չ����߳�
			return;
		}
		::ThreadWorker* pWorker = new ::ThreadWorker(worker);
		TRACE("new pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
		m_worker.store(pWorker); // ���ù����߳�
	}

	//true��ʾ���� false��ʾ�Ѿ������˹���
	bool IsIdle() { // �ж��߳��Ƿ����
		if (m_worker.load() == NULL)return true;
		return !m_worker.load()->IsValid();
	}
private:
	void ThreadWorker() { // �̹߳������������ϼ�鹤���̣߳���Ϊ˽�к�������Ϊֻ�������ڲ�����
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
	static void ThreadEntry(void* arg) { // �߳���ں���
		EdoyunThread* thiz = (EdoyunThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread; // �߳̾��
	bool m_bStatus;//false ��ʾ�߳̽�Ҫ�ر�  true ��ʾ�߳���������
	std::atomic<::ThreadWorker*> m_worker; //�����߳�,ԭ�Ӳ���
};

class EdoyunThreadPool //�̳߳���
{
public:
	EdoyunThreadPool(size_t size) { // ���캯��
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
			m_threads[i] = new EdoyunThread(); // �����߳�
	}
	EdoyunThreadPool() {}
	~EdoyunThreadPool() {
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++) {
			delete m_threads[i]; // ɾ���߳�
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}
	bool Invoke() { // �����̳߳�
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) { // �����߳�
			if (m_threads[i]->Start() == false) { // �����߳�
				ret = false;
				break;
			}
		}
		if (ret == false) {
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop(); // ֹͣ�߳�
			}
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}

	//����-1 ��ʾ����ʧ�ܣ������̶߳���æ ���ڵ���0����ʾ��n���̷߳��������������
	int DispatchWorker(const ThreadWorker& worker) { // ���乤���߳�
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) { // ����߳̿���
				m_threads[i]->UpdateWorker(worker); // ���¹����߳�
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) { // ����߳��Ƿ���Ч
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<EdoyunThread*> m_threads;
};