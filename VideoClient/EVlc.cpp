#include "pch.h"
#include "EVlc.h"

EVlc::EVlc()
{
	m_instance = libvlc_new(0, NULL);//初始化VLC
	m_media = NULL; // 初始化媒体
	m_player = NULL; // 初始化播放器
	m_hwnd = NULL; // 初始化窗口句柄
}

EVlc::~EVlc()
{
	if (m_player != NULL) {
		libvlc_media_player_t* temp = m_player; // 释放播放器
		m_player = NULL; // 置空，防止重复释放
		libvlc_media_player_release(temp); // 释放

	}
	if (m_media != NULL) {
		libvlc_media_t* temp = m_media; // 释放媒体
		m_media = NULL; // 置空
		libvlc_media_release(temp); // 释放
	}
	if (m_instance != NULL) {
		libvlc_instance_t* temp = m_instance; // 释放VLC
		m_instance = NULL; // 置空
		libvlc_release(temp); // 释放
	}
}

int EVlc::SetMedia(const std::string& strUrl)//设置视频地址
{
	if (m_instance == NULL || (m_hwnd == NULL))return -1;
	if (strUrl == m_url)return 0;
	m_url = strUrl;
	if (m_media != NULL) {
		libvlc_media_release(m_media); // 释放媒体
		m_media = NULL;
	}
	m_media = libvlc_media_new_location(m_instance, strUrl.c_str()); // 创建媒体
	if (!m_media) return -2;
	if (m_player != NULL) {
		libvlc_media_player_release(m_player); // 释放播放器
		m_player = NULL;
	}
	m_player = libvlc_media_player_new_from_media(m_media); // 创建播放器
	if (!m_player)return -3;
	CRect rect;
	GetWindowRect(m_hwnd, rect);//获取窗口大小
	std::string strRatio = "";
	strRatio.resize(32);
	sprintf((char*)strRatio.c_str(), "%d:%d", rect.Width(), rect.Height());
	libvlc_video_set_aspect_ratio(m_player, strRatio.c_str());//设置视频比例
	libvlc_media_player_set_hwnd(m_player, m_hwnd);//设置窗口句柄
	return 0;
}

#ifdef WIN32
int EVlc::SetHwnd(HWND hWnd)//设置窗口句柄
{
	m_hwnd = hWnd;
	return 0;
}
#endif

int EVlc::Play()//播放
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_media_player_play(m_player);//播放
}

int EVlc::Pause()//暂停
{
	if (!m_player || !m_instance || !m_media)return -1;
	libvlc_media_player_pause(m_player);
	return 0;
}

int EVlc::Stop()//停止
{
	if (!m_player || !m_instance || !m_media)return -1;
	libvlc_media_player_stop(m_player);
	return 0;
}

float EVlc::GetPostion()//获取进度
{
	if (!m_player || !m_instance || !m_media)return -1.0;
	return libvlc_media_player_get_position(m_player);
}

void EVlc::SetPostion(float pos)//设置进度
{
	if (!m_player || !m_instance || !m_media)return;
	libvlc_media_player_set_position(m_player, pos);
}

int EVlc::GetVolume()//获取音量
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_audio_get_volume(m_player);;
}

int EVlc::SetVolume(int volume)//设置音量
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_audio_set_volume(m_player, volume);
}

VlcSize EVlc::GetMediaInfo()//获取视频信息
{
	if (!m_player || !m_instance || !m_media)return VlcSize(-1, -1);
	return VlcSize(
		libvlc_video_get_width(m_player),
		libvlc_video_get_height(m_player)
	);
}

float EVlc::GetLength()//获取视频长度
{
	if (!m_player || !m_instance || !m_media)return -1.0f;
	libvlc_time_t tm = libvlc_media_player_get_length(m_player);
	float ret = tm / 1000.0f;
	return ret;
}

std::string EVlc::Unicode2Utf8(const std::wstring& strIn)//Unicode转UTF-8，用于支持中文
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), length, NULL, NULL);
	return str;
}
