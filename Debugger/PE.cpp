#include "PE.h"
#include <tchar.h>
#include "psapi.h"
#include <wchar.h>

DWORD PE::RvaToFoa(char * pFileData, DWORD dwRva)
{
	// �õ�����ͷ����
	// �õ�dosͷ
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileData;

	// �õ�Ntͷ
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + pFileData);

	// ʹ�ú��������һ������ͷ
	PIMAGE_SECTION_HEADER pScnHdr = IMAGE_FIRST_SECTION(pNt);

	// ���θ���
	for (int i = 0; i < pNt->FileHeader.NumberOfSections; ++i)
	{
		if (dwRva >= pScnHdr[i].VirtualAddress
			&&
			dwRva < pScnHdr[i].VirtualAddress + pScnHdr[i].SizeOfRawData)
		{
			// foa = rva - �ڴ����rva + �ļ�����foa
			DWORD dwFoa = dwRva - pScnHdr[i].VirtualAddress + pScnHdr[i].PointerToRawData;
			return dwFoa;
		}
	}
	return 0;
}

char * PE::OpenFile(PCSTR lpFilePath)
{
	FILE* pFile = NULL;
	//���ļ�
	fopen_s(&pFile, lpFilePath, "rb");
	if (!pFile)
	{
		printf("�ļ���ʧ��\n");
		getchar();
	}
	// ��ȡ�ļ���С
	fseek(pFile, 0, SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);
	//��ȡ�ļ�����
	char* pBuff = new char[fileSize];
	fread(pBuff, 1, fileSize, pFile);
	fclose(pFile);

	return pBuff;
}

void PE::EnumImport(char *pBuff)
{
	// �õ�dosͷ
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBuff;
	// �õ�NTͷ
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pDos->e_lfanew + pBuff);

	// �õ���չͷ����Ŀ¼���е��±�Ϊ1�����VirtualAddress
	DWORD dwRvaOfImp = pNt->OptionalHeader.DataDirectory[1].VirtualAddress;

	// VirtualAddress ��һ��RVA ��Ҫ��֮ת����FOA
	DWORD dwFoaOfImp = RvaToFoa(pBuff, dwRvaOfImp);

	// ���ݵ�����FOA��λ�ļ������еĵ����ṹ������
	IMAGE_IMPORT_DESCRIPTOR* pImpDir = (IMAGE_IMPORT_DESCRIPTOR*)(dwFoaOfImp + pBuff);

	// �������������
	while (pImpDir->Name)
	{
		// ����Ҫ�����dll������
		DWORD dwFoaOfDllName = RvaToFoa(pBuff, pImpDir->Name);
		char* pszDllName = dwFoaOfDllName + pBuff;
		printf("DLL��%s\n", pszDllName);

		// ����ÿ��DLL�зֱ���ĺ�������
		// ����IAT��INT
		DWORD dwFoaOfINT = RvaToFoa(pBuff, pImpDir->OriginalFirstThunk);
		DWORD dwFoaOfIAT = RvaToFoa(pBuff, pImpDir->FirstThunk);
		IMAGE_THUNK_DATA* pInt = (IMAGE_THUNK_DATA*)(dwFoaOfINT + pBuff);
		IMAGE_THUNK_DATA* pIat = (IMAGE_THUNK_DATA*)(dwFoaOfIAT + pBuff);
		printf("IAT:%08x INT:%08x\n",
			pImpDir->FirstThunk,
			pImpDir->OriginalFirstThunk);
		printf("�±� | INT���� | IAT���� | ��� | ����\n");
		int i = 0;
		while (pInt->u1.Function)
		{
			int ord = 0;
			const char* pFunName = NULL;
			// �õ����뺯�������ƺ����
			if (IMAGE_SNAP_BY_ORDINAL(pInt->u1.Ordinal))
			{
				// ������ŷ�ʽ����
				// ȡ������ֽ���Ϊ��ŵ�ֵ
				ord = pInt->u1.Ordinal & 0xFFFF;
				pFunName = "��";
			}
			else
			{
				// ����ź����Ʒ�ʽ����
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
	// �õ�dosͷ
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBuff;
	// �õ�NTͷ
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pDos->e_lfanew + pBuff);

	// �õ���չͷ����Ŀ¼���е��±�Ϊ0���VirtualAddress��������
	DWORD dwRvaOfExp = pNt->OptionalHeader.DataDirectory[0].VirtualAddress;

	// VirtualAddress ��һ��RVA ��Ҫ��֮ת��ΪFOA
	DWORD dwFoaOfExp = RvaToFoa(pBuff, dwRvaOfExp);

	// ���ݵ������FOA��λ�ļ������еĵ�����ṹ��
	IMAGE_EXPORT_DIRECTORY* pExpDir = (IMAGE_EXPORT_DIRECTORY*)(dwFoaOfExp + pBuff);

	// �õ�dll������ ��RVA��
	DWORD dwFoaOfName = RvaToFoa(pBuff, pExpDir->Name);
	char* pszDllName = dwFoaOfName + pBuff;
	printf("DLL����%s\n", pszDllName);

	// �õ������������ܸ���
	printf("������ַ������%d\n", pExpDir->NumberOfFunctions);

	// �õ������Ʒ�ʽ�����ĺ����ĸ���
	printf("��������������%d\n", pExpDir->NumberOfNames);

	// �õ�������
	// ������ַ��
	DWORD dwFoaOfEAT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	DWORD* pEat = (DWORD*)(dwFoaOfEAT + pBuff);

	// �������Ʊ�
	DWORD dwFoaOfENT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	DWORD* pEnt = (DWORD*)(dwFoaOfENT + pBuff);

	// �������Ƶ���ű�
	DWORD dwFoaOfEOT = RvaToFoa(pBuff, pExpDir->AddressOfFunctions);
	WORD* pEot = (WORD*)(dwFoaOfEOT + pBuff);

	printf("������ַ��\n");
	for (int i = 0; i < pExpDir->NumberOfFunctions; ++i)
	{
		printf("[%d] %08x\n", i, pEat[i]);
	}

	printf("�������Ʊ�\n");
	for (int i = 0; i < pExpDir->NumberOfNames; ++i)
	{
		char* pName = RvaToFoa(pBuff, pEnt[i]) + pBuff;
		printf("[%d] %08x %s\n", i, pEnt[i], pName);
	}

	// ��������ַ�������ƵĶ�Ӧ��ϵ��ӡ����
	printf("��� | ������ַ | ��������\n");
	for (int i = 0; i < pExpDir->NumberOfFunctions; ++i)
	{
		printf("%4d | %08x | ", i, pEat[i]);
		// �ж������ַ����û�����ƣ�����У��Ͱ�����������ʾ�ڸ���
		// �ж����ݣ����Ƶ���ű���û�е�ǰ��ַ���±�
		int j = 0;
		for (; j < pExpDir->NumberOfNames; ++i)
		{
			if (pEot[j] == i/*i�ǵ�ַ����±�*/)
			{
				break;
			}
		}
		if (j >= pExpDir->NumberOfNames)
		{
			printf("������\n");
		}
		else
		{
			// ��ӡ����
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
		MessageBox(0, L"����ʧ��", 0, 0);
		if (GetLastError() == 0x00000050)
		{
			MessageBox(0, L"�ļ��Ѵ���", 0, 0);
		}
		return;
	}

	IMAGE_DOS_HEADER imageDos;
	IMAGE_NT_HEADERS imageNt;
	LPVOID lpBase = (LPVOID)0x400000;

	// dosͷ
	if (ReadProcessMemory(hProcess,
		(BYTE*)lpBase, &imageDos, sizeof(IMAGE_DOS_HEADER),
		NULL) == FALSE)
		return;

	// ntͷ
	if (ReadProcessMemory(hProcess, (BYTE*)lpBase + imageDos.e_lfanew,
		&imageNt, sizeof(IMAGE_NT_HEADERS), NULL) == FALSE)
		return;

	// ��ȡ���鲢���������С
	DWORD dwNum = imageNt.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSection = new IMAGE_SECTION_HEADER[dwNum];

	// ��ȡ����
	if (ReadProcessMemory(hProcess, (BYTE*)lpBase + imageDos.e_lfanew
		+ sizeof(IMAGE_NT_HEADERS), pSection, dwNum * sizeof(IMAGE_SECTION_HEADER),
		NULL) == FALSE)
	{
		return;
	}

	// ������������Ĵ�С
	DWORD dwAllSectionSize = 0;
	DWORD dwMaxSection = 0;		// ��������

	for (int i = 0; i < dwNum; i++)
	{
		dwAllSectionSize += pSection[i].SizeOfRawData;
	}

	// �����ܴ�С
	DWORD dwSize = dwNum * sizeof(IMAGE_SECTION_HEADER) +
		sizeof(IMAGE_NT_HEADERS) + imageDos.e_lfanew;

	// ʹͷ��С�����ļ�����
	if ((dwSize & imageNt.OptionalHeader.FileAlignment) != dwSize)
	{
		dwSize &= imageNt.OptionalHeader.FileAlignment;
		dwSize += imageNt.OptionalHeader.FileAlignment;
	}

	DWORD dwFtSize = dwSize + dwAllSectionSize;

	// �����ļ�ӳ��
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
		0, dwFtSize, 0);

	if (hMap == NULL)
	{
		printf("�����ļ�ӳ��ʧ��\n");
		return;
	}

	// ������ͼ
	LPVOID lpMem = MapViewOfFile(hMap,FILE_MAP_READ|FILE_MAP_WRITE,
		0, 0, 0);

	PBYTE pbyteMem = (PBYTE)lpMem;

	memcpy(lpMem, &imageDos, sizeof(IMAGE_DOS_HEADER));

	// ����dossub��С
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
	// ����ntͷ
	memcpy(pbyteMem + imageDos.e_lfanew, &imageNt, sizeof(IMAGE_NT_HEADERS));
	// ��������
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
		printf("���浽�ļ�ʧ��\n");
		return;
	}
	delete[] pSection;
	CloseHandle(hMap);
	UnmapViewOfFile(lpMem);
	MessageBox(0, L"dump�ɹ�", 0, MB_OK);
	return;
}