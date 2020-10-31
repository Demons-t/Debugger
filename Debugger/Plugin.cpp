#include "Plugin.h"

vector<PLUGIN_INFO> Plugin::pluginList;

// ���庯��ָ��
using PfuncInitPlugin = bool(*)(int, PPLUGIN_INFO);
using freePlugin = void(*)();
using RunPlugin = void(*)(HANDLE hProcess);

void Plugin::OnInit()
{
	WIN32_FIND_DATAA info = { 0 };

	// �����׺Ϊ dll ���ļ�
	HANDLE find = FindFirstFileA("./plugin/*.dll", &info);

	// �����һ���ļ����ҳɹ����ͼ������к����ļ��ı���
	if (find != INVALID_HANDLE_VALUE)
	{
		do
		{
			// ƴ��ģ���·��
			string path = string("./plugin/") + string(info.cFileName);

			// ����Ŀ��ģ�飬
			HMODULE module = LoadLibraryA(path.c_str());

			if (module != NULL)
			{
				// �ж�Ŀ���Ƿ������Ҫ�ĵ�������
				auto initPlugin = (PfuncInitPlugin)GetProcAddress(module, "initPlugin");
				
				// ������ڣ������������
				if (initPlugin != nullptr)
				{
					PLUGIN_INFO pluginInfo = { module };
					if (initPlugin(MAKEWORD(0x00, 0x01), &pluginInfo))
					{
						pluginList.push_back(pluginInfo);
						printf("���: [%s %s] �Ѿ���������\n",
							pluginInfo.name, pluginInfo.version);
					}
				}
			}
		} while (FindNextFileA(find, &info));
	}
}

// �˳�������������ж�ز��
void Plugin::OnExit()
{
	// ��������б��ÿһ��Ԫ��
	for (const auto& plugin : pluginList)
	{
		auto free_plugin = (freePlugin)
			GetProcAddress(plugin.handle, "FreePlugin");

		// ���Ŀ�����ṩ�����������͵��ã����򲻲���
		if (free_plugin) free_plugin();

		// �ͷŲ����ģ��
		FreeLibrary(plugin.handle);
	}
}