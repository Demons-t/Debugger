#include "PE.h"
#include <tchar.h>
#include "psapi.h"
#include <wchar.h>

DWORD PE::RvaToFoa(char * pFileData, DWORD dwRva)
{
	// 得到区段头数组
	// 得到dos头
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileData;

	// 得到Nt头
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + pFileData);

	// 使用宏来求出第一个区段头
	PIMAGE_SECTION_HEADER pScnHdr = IMAGE_FIRST_SECTION(pNt);

	// 区段个数
	for (int i = 0; i < pNt->FileHeader.NumberOfSections; ++i)
	{
		if (dwRva >= pScnHdr[i].VirtualAddress
			&&
			dwRva < pScnHdr[i].VirtualAddress + pScnHdr[i].SizeOfRawData)
		{
			// foa = rva - 内存段首rva + 文件段首foa
			DWORD dwFoa = dwRva - pScnHdr[i].VirtualAddress + pScnHdr[i].PointerToRawData;
			return dwFoa;
		}
	}
	return 0;
}

char * PE::OpenFile(PCSTR lpFilePath)
{
	FILE* pFile = NULL;
	//打开文件
	fopen_s(&pFile, lpFilePath, "rb");
	if (!pFile)
	{
		printf("文件打开失败\n");
		getchar();
	}
	// 获取文件大小
	fseek(pFile, 0, SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);
	//读取文件内容
	char* pBuff = new char[fileSize];
	fread(pBuff, 1, fileSize, pFile);
	fclose(pFile);

	return pBuff;
}

void PE::EnumImport(char *pBuff)
{
	// 得到dos头
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBuff;
	// 得到NT头
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pDos->e_lfanew + pBuff);

	// 得到扩展头数据目录表中的下标为1的项的VirtualAddress
	DWORD dwRvaOfImp = pNt->OptionalHeader.DataDirectory[1].VirtualAddress;

	// VirtualAddress 是一个RVA 需要将之转换成FOA
	DWORD dwFoaOfImp = RvaToFoa(pBuff, dwRvaOfImp);

	// 根据导入表的FOA定位文件数据中的导入表结构体数组
	IMAGE_IMPORT_DESCRIPTOR* pImpDir = (IMAGE_IMPORT_DESCRIPTOR*)(dwFoaOfImp + pBuff);

	// 遍历导入表数组
	while (pImpDir->Name)
	{
		// 解析要导入的dll的名字
		DWORD dwFoaOfDllName = RvaToFoa(pBuff, pImpDir->Name);
		char* pszDllName = dwFoaOfDllName + pBuff;
		printf("DLL：%s\n", pszDllName);

		// 解析每个DLL中分别导入的函数名称
		// 遍历IAT或INT
		DWORD dwFoaOfINT = RvaToFoa(pBuff, pImpDir->OriginalFirstThunk);
		DWORD dwFoaOfIAT = RvaToFoa(pBuff, pImpDir->FirstThunk);
		IMAGE_THUNK_DATA* pInt = (IMAGE_THUNK_DATA*)(dwFoaOfINT + pBuff);
		IMAGE_THUNK_DATA* pIat = (IMAGE_THUNK_DATA*)(dwFoaOfIAT + pBuff);
		printf("IAT:%08x INT:%08x\n",
			pImpDir->FirstThunk,
			pImpDir->OriginalFirstThunk);
		printf("下标 | INT数据 | IAT数据 | 序号 | 名称\n");
		int i = 0;
		while (pInt->u1.Function)
		{
			int ord = 0;
			const char* pFunName = NULL;
			// 得到导入函数的名称和序号
			if (IMAGE_SNAP_BY_ORDINAL(pInt->u1.Ordinal))
			{
				// 仅以序号方式导入
				// 取其低两字节作为序号的值
				ord = pInt->u1.Ordinal & 0xFFFF;
				pFunName = "无";
			}
			else
			{
				// 以序号和名称方式导入
				DWORD dwFoaOfNameStruct = RvaToFoa(pBuff, pInt->u1.AddressOfData);
				IMAGE_IMPORT_BY_NAME* pImpName = (IMAGE_IMPORT_BY_NAME*)
					(dwFoaOfNameStruct + pBuff);
				ord = pImpName->Hint;
				pFunName = pImpName->Name;
			}
			printf("%04d | %08x | %08x | %04x | %s\n",
				i, pInt->u1.Function, pIat->u1.Function, ord, pFunName);
			++pInt;
			++pIat;
			++i;
		}
		++pImpDir;
	}

	delete[] pBuff;
}

void PE::EnumExport(char * pBuff)
{
	// 得到dos头
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBuff;
	// 得到NT头
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pDos->e_lfanew + pBuff);

	// 得到扩展头数据目录表中的下标为0项的VirtualAddress（导出表）
	DWORD dwRvaOfExp = pNt->OptionalHeader.DataDirectory[0].VirtualAddress;

	// VirtualAddress 是一个RVA 需要将之转换为FOA
	DWORD dwFoaOfExp = RvaToFoa(pBuff, dwRvaOfExp);

	// 根据导出表的FOA定位文件数据中的导出表结构体
	IMAGE_EXPORT_DIRECTORY* pExpDir = (IMAGE_EXPORT_DIRECTORY*)(dwFoaOfExp + pBuff);

	// 得到dll的名字 （RVA）
	DWORD dwFoaOfName = RvaToFoa(pBuff, pExpDir->Name);
	char* pszDllName = dwFoaOfName + pBuff;
	printf("DLL名：%s\n", pszDllName);

	// 得到导出函数的总个数
	printf("导出地址总数：%d\n", pExpDir->NumberOfFunctions);

	// 得到以名称方式导出的函数的个数
	printf("导出名称总数：%d\n", pExpDir->NumberOfNames);

	// 得到三个表
	// 导出地址表
	DWORD dwFoaOfEAT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	DWORD* pEat = (DWORD*)(dwFoaOfEAT + pBuff);

	// 导出名称表
	DWORD dwFoaOfENT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	DWORD* pEnt = (DWORD*)(dwFoaOfENT + pBuff);

	// 导出名称的序号表
	DWORD dwFoaOfEOT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	WORD* pEot = (WORD*)(dwFoaOfEOT + pBuff);

	printf("导出地址表：\n");
	for (int i = 0; i < pExpDir->NumberOfFunctions; ++i)
	{
		printf("[%d] %08x\n", i, pEat[i]);
	}

	printf("导出名称表：\n");
	for (int i = 0; i < pExpDir->NumberOfNames; ++i)
	{
		char* pName = RvaToFoa(pBuff, pEnt[i]) + pBuff;
		printf("[%d] %08x %s\n", i, pEnt[i], pName);
	}

	// 将函数地址函数名称的对应关系打印出来
	printf("序号 | 函数地址 | 函数名称\n");
	for (int i = 0; i < pExpDir->NumberOfFunctions; ++i)
	{
		printf("%4d | %08x | ", i, pEat[i]);
		// 判断这个地址，有没有名称，如果有，就把它的名称显示在该行
		// 判断依据：名称的序号表有没有当前地址的下标
		int j = 0;
		for (; j < pExpDir->NumberOfNames; ++i)
		{
			if (pEot[j] == i/*i是地址表的下标*/)
			{
				break;
			}
		}
		if (j >= pExpDir->NumberOfNames)
		{
			printf("无名称\n");
		}
		else
		{
			// 打印名称
			char* pName = RvaToFoa(pBuff, pEnt[j]) + pBuff;
			printf("%s\n", pName);
		}
	}

	delete[] pBuff;
}

void PE::Dump(const char * pName, HANDLE hProcess)
{
	PCSTR strName = pName;
	HANDLE hFile = CreateFileA(strName, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, L"创建失败", 0, 0);
		if (GetLastError() == 0x00000050)
		{
			MessageBox(0, L"文件已存在", 0, 0);
		}
		return;
	}

	IMAGE_DOS_HEADER imageDos;
	IMAGE_NT_HEADERS imageNt;
	LPVOID lpBase = (LPVOID)0x400000;

	// dos头
	if (ReadProcessMemory(hProcess,
		(BYTE*)lpBase, &imageDos, sizeof(IMAGE_DOS_HEADER),
		NULL) == FALSE)
		return;

	// nt头
	if (ReadProcessMemory(hProcess, (BYTE*)lpBase + imageDos.e_lfanew,
		&imageNt, sizeof(IMAGE_NT_HEADERS), NULL) == FALSE)
		return;

	// 读取区块并计算区块大小
	DWORD dwNum = imageNt.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSection = new IMAGE_SECTION_HEADER[dwNum];

	// 读取区块
	if (ReadProcessMemory(hProcess, (BYTE*)lpBase + imageDos.e_lfanew
		+ sizeof(IMAGE_NT_HEADERS), pSection, dwNum * sizeof(IMAGE_SECTION_HEADER),
		NULL) == FALSE)
	{
		return;
	}

	// 计算所有区块的大小
	DWORD dwAllSectionSize = 0;
	DWORD dwMaxSection = 0;		// 最大的区块

	for (int i = 0; i < dwNum; i++)
	{
		dwAllSectionSize += pSection[i].SizeOfRawData;
	}

	// 区块总大小
	DWORD dwSize = dwNum * sizeof(IMAGE_SECTION_HEADER) +
		sizeof(IMAGE_NT_HEADERS) + imageDos.e_lfanew;

	// 使头大小按照文件对齐
	if ((dwSize & imageNt.OptionalHeader.FileAlignment) != dwSize)
	{
		dwSize &= imageNt.OptionalHeader.FileAlignment;
		dwSize += imageNt.OptionalHeader.FileAlignment;
	}

	DWORD dwFtSize = dwSize + dwAllSectionSize;

	// 创建文件映射
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
		0, dwFtSize, 0);

	if (hMap == NULL)
	{
		printf("创建文件映射失败\n");
		return;
	}

	// 创建视图
	LPVOID lpMem = MapViewOfFile(hMap,FILE_MAP_READ|FILE_MAP_WRITE,
		0, 0, 0);

	PBYTE pbyteMem = (PBYTE)lpMem;

	memcpy(lpMem, &imageDos, sizeof(IMAGE_DOS_HEADER));

	// 计算dossub大小
	DWORD dwSubSize = imageDos.e_lfanew - sizeof(IMAGE_DOS_HEADER);
	if (ReadProcessMemory(hProcess, (BYTE*)lpBase + sizeof(IMAGE_DOS_HEADER),
		pbyteMem+sizeof(IMAGE_DOS_HEADER), dwSize, NULL) == FALSE)
	{
		delete[] pSection;
		CloseHandle(hMap);
		UnmapViewOfFile(lpMem);
		return;
	}

	imageNt.OptionalHeader.ImageBase = (DWORD)lpBase;
	// 保存nt头
	memcpy(pbyteMem + imageDos.e_lfanew, &imageNt, sizeof(IMAGE_NT_HEADERS));
	// 保存区块
	memcpy(pbyteMem + imageDos.e_lfanew + sizeof(IMAGE_NT_HEADERS),
		pSection, dwNum * sizeof(IMAGE_SECTION_HEADER));

	for (int i = 0; i < dwNum; i++)
	{
		if (ReadProcessMemory(hProcess,
			(BYTE*)lpBase + pSection[i].VirtualAddress,
			pbyteMem + pSection[i].PointerToRawData,
			pSection[i].SizeOfRawData, NULL) == FALSE)
		{
			delete[] pSection;
			CloseHandle(hMap);
			UnmapViewOfFile(lpMem);
			return;
		}
	}

	if (FlushViewOfFile(lpMem, 0) == false)
	{
		delete[] pSection;
		CloseHandle(hMap);
		UnmapViewOfFile(lpMem);
		printf("保存到文件失败\n");
		return;
	}
	delete[] pSection;
	CloseHandle(hMap);
	UnmapViewOfFile(lpMem);
	MessageBox(0, L"dump成功", 0, MB_OK);
	return;
}