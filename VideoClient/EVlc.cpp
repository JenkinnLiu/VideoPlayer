#include "pch.h"
#include "EVlc.h"

EVlc::EVlc()
{
	m_instance = libvlc_new(0, NULL);//��ʼ��VLC
	m_media = NULL; // ��ʼ��ý��
	m_player = NULL; // ��ʼ��������
	m_hwnd = NULL; // ��ʼ�����ھ��
}

EVlc::~EVlc()
{
	if (m_player != NULL) {
		libvlc_media_player_t* temp = m_player; // �ͷŲ�����
		m_player = NULL; // �ÿգ���ֹ�ظ��ͷ�
		libvlc_media_player_release(temp); // �ͷ�

	}
	if (m_media != NULL) {
		libvlc_media_t* temp = m_media; // �ͷ�ý��
		m_media = NULL; // �ÿ�
		libvlc_media_release(temp); // �ͷ�
	}
	if (m_instance != NULL) {
		libvlc_instance_t* temp = m_instance; // �ͷ�VLC
		m_instance = NULL; // �ÿ�
		libvlc_release(temp); // �ͷ�
	}
}

int EVlc::SetMedia(const std::string& strUrl)//������Ƶ��ַ
{
	if (m_instance == NULL || (m_hwnd == NULL))return -1;
	if (strUrl == m_url)return 0;
	m_url = strUrl;
	if (m_media != NULL) {
		libvlc_media_release(m_media); // �ͷ�ý��
		m_media = NULL;
	}
	m_media = libvlc_media_new_location(m_instance, strUrl.c_str()); // ����ý��
	if (!m_media) return -2;
	if (m_player != NULL) {
		libvlc_media_player_release(m_player); // �ͷŲ�����
		m_player = NULL;
	}
	m_player = libvlc_media_player_new_from_media(m_media); // ����������
	if (!m_player)return -3;
	CRect rect;
	GetWindowRect(m_hwnd, rect);//��ȡ���ڴ�С
	std::string strRatio = "";
	strRatio.resize(32);
	sprintf((char*)strRatio.c_str(), "%d:%d", rect.Width(), rect.Height());
	libvlc_video_set_aspect_ratio(m_player, strRatio.c_str());//������Ƶ����
	libvlc_media_player_set_hwnd(m_player, m_hwnd);//���ô��ھ��
	return 0;
}

#ifdef WIN32
int EVlc::SetHwnd(HWND hWnd)//���ô��ھ��
{
	m_hwnd = hWnd;
	return 0;
}
#endif

int EVlc::Play()//����
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_media_player_play(m_player);//����
}

int EVlc::Pause()//��ͣ
{
	if (!m_player || !m_instance || !m_media)return -1;
	libvlc_media_player_pause(m_player);
	return 0;
}

int EVlc::Stop()//ֹͣ
{
	if (!m_player || !m_instance || !m_media)return -1;
	libvlc_media_player_stop(m_player);
	return 0;
}

float EVlc::GetPostion()//��ȡ����
{
	if (!m_player || !m_instance || !m_media)return -1.0;
	return libvlc_media_player_get_position(m_player);
}

void EVlc::SetPostion(float pos)//���ý���
{
	if (!m_player || !m_instance || !m_media)return;
	libvlc_media_player_set_position(m_player, pos);
}

int EVlc::GetVolume()//��ȡ����
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_audio_get_volume(m_player);;
}

int EVlc::SetVolume(int volume)//��������
{
	if (!m_player || !m_instance || !m_media)return -1;
	return libvlc_audio_set_volume(m_player, volume);
}

VlcSize EVlc::GetMediaInfo()//��ȡ��Ƶ��Ϣ
{
	if (!m_player || !m_instance || !m_media)return VlcSize(-1, -1);
	return VlcSize(
		libvlc_video_get_width(m_player),
		libvlc_video_get_height(m_player)
	);
}

float EVlc::GetLength()//��ȡ��Ƶ����
{
	if (!m_player || !m_instance || !m_media)return -1.0f;
	libvlc_time_t tm = libvlc_media_player_get_length(m_player);
	float ret = tm / 1000.0f;
	return ret;
}

std::string EVlc::Unicode2Utf8(const std::wstring& strIn)//UnicodeתUTF-8������֧������
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), length, NULL, NULL);
	return str;
}
