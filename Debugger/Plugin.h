#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>

using namespace std;

// �ṩһ���ṹ�����ڱ���������Ϣ
typedef struct _PLUGIN_INFO
{
	HMODULE handle;
	char name[0x20];
	char version[0x20];
}PLUGIN_INFO, *PPLUGIN_INFO;

class Plugin
{
private:
	static vector<PLUGIN_INFO> pluginList;		// ����һ���������ڱ������еĲ����Ϣ

public:
	static void OnInit();						// ��ʼ��
	static void OnExit();
};

