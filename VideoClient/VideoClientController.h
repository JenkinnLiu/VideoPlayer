#pragma once
#include "EVlc.h"
#include "VideoClientDlg.h"
enum EVlcCommand {//VLC命令
	EVLC_PLAY,
	EVLC_PAUSE,
	EVLC_STOP,
	EVLC_GET_VOLUME,
	EVLC_GET_POSITION,
	EVLC_GET_LENGTH
};
class VideoClientController //视频客户端控制层
{
public:
	VideoClientController();
	~VideoClientController();
	int Init(CWnd*& pWnd);
	int Invoke();//启动视频客户端
	//如果strUrl中包含中文符号或字符，则使用utf-8编码
	int SetMedia(const std::string& strUrl);
	//返回-1.0表示错误
	float VideoCtrl(EVlcCommand cmd);//控制视频播放
	void SetPosition(float pos);//设置视频进度
	int SetWnd(HWND hWnd);//设置窗口句柄
	int SetVolume(int volume);//设置音量
	VlcSize GetMediaInfo();//获取视频信息
	std::string Unicode2Utf8(const std::wstring& strIn);
protected:
	EVlc m_vlc;//VLC播放器
	CVideoClientDlg m_dlg;//视频客户端对话框
};

