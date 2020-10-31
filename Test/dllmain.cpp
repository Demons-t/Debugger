#include "pch.h"
#include <stdio.h>
#include <Windows.h>

// 提供一个结构体用于保存插件
typedef struct _PLUGIN_INFO
{
	HMODULE handle;
	char name[0x20];
	char version[0x20];
}PLUGIN_INFO, *PPLUGIN_INFO;

// 以解除名称粉碎的方式导出函数
extern "C" _declspec(dllexport) bool initPlugin(int version, PPLUGIN_INFO pluginInfo)
{
	// 如果目标程序的版本不适配当前插件支持的版本，就返回false
	if (version != MAKEWORD(0x00, 0x01))
	{
		return false;
	}

	// 版本号匹配
	memcpy(pluginInfo->name, "Test", 5);
	memcpy(pluginInfo->version, "0.0.1", 6);

	return true;
}

extern "C" _declspec(dllexport) void FreePlugin()
{
	printf("插件清理成功\n");
}