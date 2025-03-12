#pragma once
#include "EVlc.h"
#include "VideoClientDlg.h"
enum EVlcCommand {//VLC����
	EVLC_PLAY,
	EVLC_PAUSE,
	EVLC_STOP,
	EVLC_GET_VOLUME,
	EVLC_GET_POSITION,
	EVLC_GET_LENGTH
};
class VideoClientController //��Ƶ�ͻ��˿��Ʋ�
{
public:
	VideoClientController();
	~VideoClientController();
	int Init(CWnd*& pWnd);
	int Invoke();//������Ƶ�ͻ���
	//���strUrl�а������ķ��Ż��ַ�����ʹ��utf-8����
	int SetMedia(const std::string& strUrl);
	//����-1.0��ʾ����
	float VideoCtrl(EVlcCommand cmd);//������Ƶ����
	void SetPosition(float pos);//������Ƶ����
	int SetWnd(HWND hWnd);//���ô��ھ��
	int SetVolume(int volume);//��������
	VlcSize GetMediaInfo();//��ȡ��Ƶ��Ϣ
	std::string Unicode2Utf8(const std::wstring& strIn);
protected:
	EVlc m_vlc;//VLC������
	CVideoClientDlg m_dlg;//��Ƶ�ͻ��˶Ի���
};

