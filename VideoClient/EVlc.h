#pragma once
#include <string>
#include "vlc.h"
class VlcSize { // 定义VlcSize类
public:
	int nWidth;
	int nHeight;
	VlcSize(int width = 0, int height = 0) {
		nWidth = width; // 宽度
		nHeight = height; // 高度
	}
	VlcSize(const VlcSize& size) { // 拷贝构造函数
		nWidth = size.nWidth;
		nHeight = size.nHeight;
	}
	VlcSize& operator=(const VlcSize& size) { // 重载赋值运算符
		if (this != &size) {
			nWidth = size.nWidth;
			nHeight = size.nHeight;
		}
		return *this;
	}
};
class EVlc // 定义EVlc类
{
public:
	EVlc(); // 构造函数
	~EVlc();
	//strUrl如果包含中文，请传入utf-8的编码格式字符串
	int SetMedia(const std::string& strUrl); // 设置视频地址
#ifdef WIN32
	int SetHwnd(HWND hWnd); // 设置窗口句柄
#endif
	int Play(); // 播放
	int Pause(); // 暂停
	int Stop(); // 停止
	float GetPostion(); // 获取进度
	void SetPostion(float pos); // 设置进度
	int GetVolume(); // 获取音量
	int SetVolume(int volume); // 设置音量
	VlcSize GetMediaInfo(); // 获取视频信息
	float GetLength(); // 获取视频长度
	std::string Unicode2Utf8(const std::wstring& strIn);	// Unicode转UTF-8
protected:
	libvlc_instance_t* m_instance; // VLC实例
	libvlc_media_t* m_media; // VLC媒体
	libvlc_media_player_t* m_player; // VLC播放器
	std::string m_url; // 视频地址
#ifdef WIN32
	HWND m_hwnd;
#endif
};

