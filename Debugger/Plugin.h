#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>

using namespace std;

// 提供一个结构体用于保存插件的信息
typedef struct _PLUGIN_INFO
{
	HMODULE handle;
	char name[0x20];
	char version[0x20];
}PLUGIN_INFO, *PPLUGIN_INFO;

class Plugin
{
private:
	static vector<PLUGIN_INFO> pluginList;		// 创建一个容器用于保存所有的插件信息

public:
	static void OnInit();						// 初始化
	static void OnExit();
};

