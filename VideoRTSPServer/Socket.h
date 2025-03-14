#pragma once
#include <WinSock2.h>
#include <memory>
#include "base.h"

#pragma warning(disable:6031)
#pragma comment(lib, "ws2_32.lib") // 网络api函数



class Socket
{
public:
	//nType 0 TCP 1 UDP
	Socket(bool bIsTcp = true) {// 构造函数
		m_sock = INVALID_SOCKET; 
		if (bIsTcp) { // 创建一个TCP套接字
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else { // 创建一个UDP套接字
			m_sock = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}
	Socket(SOCKET s) { 
		m_sock = s;
	}
	/*
	这种关闭套接字的方法有以下几个好处：
	1.	防止重复关闭：通过将 m_sock 设置为 INVALID_SOCKET，可以防止同一个套接字被重复关闭。重复关闭可能会导致未定义行为或错误。
	2.	资源管理：在关闭套接字之前，将 m_sock 的值保存到临时变量 temp 中，然后将 m_sock 设置为 INVALID_SOCKET。
	这样可以确保在 closesocket 调用过程中，即使发生异常或错误，m_sock 也不会再指向有效的套接字，从而避免资源泄漏。
	3.	线程安全：虽然这个方法本身并没有完全解决线程安全问题，但通过将 m_sock 设置为 INVALID_SOCKET，
	可以减少多线程环境下的竞争条件。如果多个线程尝试关闭同一个套接字，设置 m_sock 为 INVALID_SOCKET 可以减少冲突的可能性。
	*/
	void Close() { // 关闭套接字
		if (m_sock != INVALID_SOCKET) {
			SOCKET temp = m_sock; 
			m_sock = INVALID_SOCKET;
			closesocket(temp);
		}
	}
	operator SOCKET() {  // 重载运算符
		return m_sock; 
	}
	~Socket() { // 析构函数
		Close();
	}
private:
	SOCKET m_sock;
};
class EAddress 
{
public:
	EAddress() {
		m_port = -1;
		memset(&m_addr, 0, sizeof(m_addr));
		m_addr.sin_family = AF_INET;
	}

	EAddress(const std::string& ip, short port) {
		m_ip = ip;
		m_port = port;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}

	EAddress(const EAddress& addr) {
		m_ip = addr.m_ip;
		m_port = addr.m_port;
		memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
	}

	EAddress& operator=(const EAddress& addr) {
		if (this != &addr) {
			m_ip = addr.m_ip;
			m_port = addr.m_port;
			memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
		}
		return *this;
	}

	EAddress& operator=(short port) {
		m_port = (unsigned short)port;
		m_addr.sin_port = htons(port);
		return *this;
	}

	~EAddress() {}

	void Update(const std::string& ip, short port) {
		m_ip = ip;
		m_port = port;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}

	operator const sockaddr* () const {
		return (sockaddr*)&m_addr;
	}

	operator sockaddr* () {
		return (sockaddr*)&m_addr;
	}

	operator sockaddr_in* () {
		return &m_addr;
	}

	int size()const { return sizeof(sockaddr_in); }

	const std::string Ip() const {
		return m_ip;
	}

	unsigned short Port()const {
		return m_port;
	}

	void Fresh() {
		m_ip = inet_ntoa(m_addr.sin_addr);
	}
private:
	std::string m_ip;
	unsigned short m_port;
	sockaddr_in m_addr;
};

class ESocket// 封装套接字
{
public:
	ESocket(bool isTcp = true) // 构造函数
		:m_socket(new Socket(isTcp)), // 创建一个套接字
		m_istcp(isTcp)
	{
	}
	ESocket(const ESocket& sock)
		:m_socket(sock.m_socket),
		m_istcp(sock.m_istcp)
	{
	}
	ESocket(SOCKET sock, bool isTcp) :
		m_socket(new Socket(sock)),
		m_istcp(isTcp)
	{
	}

	ESocket& operator=(const ESocket& sock) { // 重载运算符
		if (this != &sock)m_socket = sock.m_socket;
		return *this;
	}
	~ESocket() { // 析构函数
		m_socket.reset();
	}

	operator SOCKET() const { // 重载运算符
		return *m_socket; // 返回套接字
	}

	int Bind(const EAddress& addr) { // 绑定套接字
		if (m_socket == nullptr) {
			m_socket.reset(new Socket(m_istcp)); // 创建一个套接字
		}
		return bind(*m_socket, addr, addr.size());
	}

	int Listen(int backlog = 5) {
		return listen(*m_socket, backlog);
	}

	ESocket Accept(EAddress& addr) {
		int len = addr.size();
		if (m_socket == nullptr)return ESocket(INVALID_SOCKET, true);
		SOCKET server = *m_socket;
		if (server == INVALID_SOCKET)return ESocket(INVALID_SOCKET, true);
		SOCKET s = accept(server, addr, &len);
		return ESocket(s, m_istcp);
	}

	int Connect(const EAddress& addr) {
		return connect(*m_socket, addr, addr.size());
	}

	int Recv(EBuffer& buffer) {
		return recv(*m_socket, buffer, buffer.size(), 0);
	}

	int Send(const EBuffer& buffer) {
		printf("send(%d):[%s]\r\n", buffer.size(), (char*)buffer);
		int index = 0;
		char* pData = buffer;
		while (index < (int)buffer.size()) {
			int ret = send(*m_socket, pData + index, buffer.size() - index, 0);
			if (ret < 0)return ret;
			if (ret == 0)break;
			index += ret;
		}
		return index;
	}

	void Close() {
		m_socket.reset();
	}

private:
	std::shared_ptr<Socket> m_socket;// 智能指针管理套接字：1. 防止内存泄漏 2. 防止重复释放内存， 3. 线程安全
	bool m_istcp;
};

class SocketIniter
{
public:
	SocketIniter() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~SocketIniter() {
		WSACleanup();
	}
};