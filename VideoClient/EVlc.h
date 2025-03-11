#pragma once
#include <string>
#include "vlc.h"
class VlcSize { // ����VlcSize��
public:
	int nWidth;
	int nHeight;
	VlcSize(int width = 0, int height = 0) {
		nWidth = width; // ���
		nHeight = height; // �߶�
	}
	VlcSize(const VlcSize& size) { // �������캯��
		nWidth = size.nWidth;
		nHeight = size.nHeight;
	}
	VlcSize& operator=(const VlcSize& size) { // ���ظ�ֵ�����
		if (this != &size) {
			nWidth = size.nWidth;
			nHeight = size.nHeight;
		}
		return *this;
	}
};
class EVlc // ����EVlc��
{
public:
	EVlc(); // ���캯��
	~EVlc();
	//strUrl����������ģ��봫��utf-8�ı����ʽ�ַ���
	int SetMedia(const std::string& strUrl); // ������Ƶ��ַ
#ifdef WIN32
	int SetHwnd(HWND hWnd); // ���ô��ھ��
#endif
	int Play(); // ����
	int Pause(); // ��ͣ
	int Stop(); // ֹͣ
	float GetPostion(); // ��ȡ����
	void SetPostion(float pos); // ���ý���
	int GetVolume(); // ��ȡ����
	int SetVolume(int volume); // ��������
	VlcSize GetMediaInfo(); // ��ȡ��Ƶ��Ϣ
	float GetLength(); // ��ȡ��Ƶ����
	std::string Unicode2Utf8(const std::wstring& strIn);	// UnicodeתUTF-8
protected:
	libvlc_instance_t* m_instance; // VLCʵ��
	libvlc_media_t* m_media; // VLCý��
	libvlc_media_player_t* m_player; // VLC������
	std::string m_url; // ��Ƶ��ַ
#ifdef WIN32
	HWND m_hwnd;
#endif
};

