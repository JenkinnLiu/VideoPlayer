// VideoPlay.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <Windows.h>
#include <iostream>
#include "vlc.h"
#include <conio.h>
std::string Unicode2Utf8(const std::wstring& strIn) //Unicode转utf-8
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);//用于获取转换后的长度
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), length, NULL, NULL);//转换
	return str;
}

int main()
{
	int argc = 1;
	char* argv[2];
	argv[0] = (char*)"--ignore-config";//多字节转Unicode，再转utf-8
	libvlc_instance_t* vlc_ins = libvlc_new(argc, argv);//初始化vlc
	//std::string path = Unicode2Utf8(L"股市讨论.mp4");
	//libvlc_media_t* media = libvlc_media_new_path(vlc_ins, path.c_str());
	std::string path = Unicode2Utf8(L"file:///E:\\edoyun\\study_project\\VideoPlay\\VideoPlay\\股市讨论.mp4");
	libvlc_media_t* media = libvlc_media_new_location(vlc_ins, path.c_str()); //加载vlc_media
	libvlc_media_player_t* player = libvlc_media_player_new_from_media(media); //创建vlc_player播放器
	do {
		int ret = libvlc_media_player_play(player);//播放
		if (ret == -1) {
			printf("error found!\r\n");
			break;
		}
		int vol = -1;
		//只有media解析加载完成，才会有下面的参数
		while (vol == -1) {
			Sleep(10);
			vol = libvlc_audio_get_volume(player);//获取音量
		}
		printf("volume is %d\r\n", vol);
		libvlc_audio_set_volume(player, 10);//设置音量
		libvlc_time_t tm = libvlc_media_player_get_length(player);//获取视频长度
		printf("%02d:%02d:%02d.%03d\r\n", int(tm / 3600000), int(tm / 60000) % 60, int(tm / 1000) % 60, int(tm) % 1000);//打印格式化的视频长度
		int width = libvlc_video_get_width(player);
		int height = libvlc_video_get_height(player);//获取视频宽高
		printf("width=%d height=%d\r\n", width, height);
		while (!_kbhit()) {//按任意键退出
			printf("%f%%\r", 100.0 * libvlc_media_player_get_position(player));//打印播放进度
			Sleep(500);
		}
		getchar();
		libvlc_media_player_pause(player);//暂停
		getchar();
		libvlc_media_player_play(player);//继续播放
		getchar();
		libvlc_media_player_stop(player);//停止
	} while (0);
	libvlc_media_player_release(player);//释放player
	libvlc_media_release(media);//释放media
	libvlc_release(vlc_ins);//释放vlc
	return 0;
}
