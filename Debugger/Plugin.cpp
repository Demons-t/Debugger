#include "Plugin.h"

vector<PLUGIN_INFO> Plugin::pluginList;

// 定义函数指针
using PfuncInitPlugin = bool(*)(int, PPLUGIN_INFO);
using freePlugin = void(*)();
using RunPlugin = void(*)(HANDLE hProcess);

void Plugin::OnInit()
{
	WIN32_FIND_DATAA info = { 0 };

	// 处理后缀为 dll 的文件
	HANDLE find = FindFirstFileA("./plugin/*.dll", &info);

	// 如果第一个文件查找成功，就继续进行后续文件的遍历
	if (find != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 拼接模块的路径
			string path = string("./plugin/") + string(info.cFileName);

			// 加载目标模块，
			HMODULE module = LoadLibraryA(path.c_str());

			if (module != NULL)
			{
				// 判断目标是否存在需要的导出函数
				auto initPlugin = (PfuncInitPlugin)GetProcAddress(module, "initPlugin");
				
				// 如果存在，调用这个函数
				if (initPlugin != nullptr)
				{
					PLUGIN_INFO pluginInfo = { module };
					if (initPlugin(MAKEWORD(0x00, 0x01), &pluginInfo))
					{
						pluginList.push_back(pluginInfo);
						printf("插件: [%s %s] 已经被加载了\n",
							pluginInfo.name, pluginInfo.version);
					}
				}
			}
		} while (FindNextFileA(find, &info));
	}
}

// 退出程序：清理插件，卸载插件
void Plugin::OnExit()
{
	// 遍历插件列表的每一个元素
	for (const auto& plugin : pluginList)
	{
		auto free_plugin = (freePlugin)
			GetProcAddress(plugin.handle, "FreePlugin");

		// 如果目标插件提供了清理函数，就调用，否则不操作
		if (free_plugin) free_plugin();

		// 释放插件的模块
		FreeLibrary(plugin.handle);
	}
}