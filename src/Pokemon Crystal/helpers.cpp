#include "stdafx.h"

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot == INVALID_HANDLE_VALUE )
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if ( !processName.compare(processInfo.szExeFile) )
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while ( Process32Next(processesSnapshot, &processInfo) )
	{
		if ( !processName.compare(processInfo.szExeFile) )
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	
	CloseHandle(processesSnapshot);
	return 0;
}


char* GetAddressOfData(DWORD pid, const char *data, size_t len)
{
	char* lastFound = 0;
	DWORD addr;
	process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(process)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);

        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk;
		char* p = 0;
		DWORD processed = 0;
        while(p < si.lpMaximumApplicationAddress)
        {
            if(VirtualQueryEx(process, (char*)p, &info, sizeof(info)) == sizeof(info))
            {
                p = (char*)info.BaseAddress;
                chunk.resize(info.RegionSize);
                SIZE_T bytesRead;
                if(ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead))
                {
                    for(size_t i = 0; i < (bytesRead - len); ++i)
                    {
                        if(memcmp(data, &chunk[i], len) == 0)
                        {
							lastFound = (char*)p + i;
							memcpy(&addr, &lastFound, 4);
							addr -= 0x0BD2;

							gAddresses.push_back(addr);
                        }
                    }
					processed += bytesRead;
                }
                p += info.RegionSize;
            }
        }
    }

    return lastFound;
}