#pragma once
#include <string>
#include <list>

std::wstring str2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    std::wstring buf;
    buf.resize(len);
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength,
        const_cast<wchar_t*>(buf.c_str()), len);
    return buf;
}

HANDLE start_w_process(std::string logFileName) {
    STARTUPINFO startupInfo;
    SECURITY_ATTRIBUTES* securAttr = new SECURITY_ATTRIBUTES();
    securAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
    securAttr->bInheritHandle = true;
    securAttr->lpSecurityDescriptor = nullptr;
    HANDLE logFileHandle = CreateFile(str2ws(logFileName).c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, securAttr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdOutput = logFileHandle;
    startupInfo.hStdError = nullptr;
    startupInfo.hStdInput = nullptr;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION* procInfo = new PROCESS_INFORMATION();
    bool new_process = CreateProcess(str2ws("C:\\Users\\User\\source\\repos\\OS_Lab4_part1_w\\Debug\\OS_Lab4_part1_w.exe").c_str(), nullptr, nullptr, nullptr, true, 0, nullptr,
        nullptr, &startupInfo, procInfo);
    if (new_process) {
        std::cout << procInfo->dwProcessId << '\n';
        return procInfo->hProcess;
    }
    else
        std::cout << "Process is not created: " << GetLastError() << std::endl;
    return nullptr;
}

HANDLE start_r_process(std::string logFileName) {
    STARTUPINFO* startupInfo = new STARTUPINFO();
    SECURITY_ATTRIBUTES* securAttr = new SECURITY_ATTRIBUTES();
    securAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
    securAttr->bInheritHandle = true;
    securAttr->lpSecurityDescriptor = nullptr;
    HANDLE logFileHandle = CreateFile(str2ws(logFileName).c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, securAttr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (logFileHandle == INVALID_HANDLE_VALUE)
        std::cout << "Log" << GetLastError() << std::endl;
    startupInfo->cb = sizeof(STARTUPINFO);
    startupInfo->hStdOutput = logFileHandle;
    startupInfo->hStdError = nullptr;
    startupInfo->hStdInput = nullptr;
    startupInfo->dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION* procInfo = new PROCESS_INFORMATION();
    bool new_process = CreateProcess(str2ws("C:\\Users\\User\\source\\repos\\OS_Lab4_part1_r\\Debug\\OS_Lab4_part1_r.exe").c_str(), nullptr, nullptr, nullptr, true, 0, nullptr,
        nullptr, startupInfo, procInfo);
    if (new_process)
        return procInfo->hProcess;
    else
        std::cout << "Process is not created: " << GetLastError() << std::endl;
    return nullptr;
}

void winapi_start(std::vector<std::string>* args) {
	const unsigned int page_size = 4096;
	unsigned int w_count = std::stoi((*args)[0]);
	unsigned int r_count = std::stoi((*args)[1]);
	unsigned int p_count = std::stoi((*args)[2]);
    unsigned int file_size = page_size * p_count;

    HANDLE* free_semaphores = new HANDLE[p_count];
    HANDLE* recorded_semaphores = new HANDLE[p_count];
    for (int i = 0; i < p_count; i++) {
        free_semaphores[i] = CreateSemaphore(nullptr, 1, 1, str2ws("freeSemaphore" + std::to_string(i)).c_str());
        recorded_semaphores[i] = CreateSemaphore(nullptr, 0, 1, str2ws("recordedSemaphore" + std::to_string(i)).c_str());
    }

    HANDLE buffer_handle = CreateFile(str2ws("C:\\OS_Lab4\\Buffer.txt").c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    HANDLE buffer_mapping = CreateFileMapping(buffer_handle, nullptr, PAGE_READWRITE, 0, file_size, str2ws("Buffer_mapping").c_str());
    HANDLE buffer_view = MapViewOfFile(buffer_mapping, FILE_MAP_ALL_ACCESS, 0, 0, file_size);

    HANDLE p_count_handle = CreateFile(str2ws("C:\\OS_Lab4\\PagesCount.txt").c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    HANDLE p_count_mapping = CreateFileMapping(p_count_handle, nullptr, PAGE_READWRITE, 0, 12, str2ws("Pages_count_mapping").c_str());
    HANDLE p_count_view = MapViewOfFile(p_count_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 12);
    memcpy(p_count_view, &p_count, 4);
    memcpy((char*)p_count_view + 4, &w_count, 4);
    memcpy((char*)p_count_view + 8, &r_count, 4);

    VirtualLock(buffer_view, file_size);
    VirtualLock(buffer_view, 12);

    HANDLE* proc_handles = new HANDLE[w_count + r_count];
    for (int i = 0; i < w_count; i++)
    {
        std::string logFileName = std::string("C:\\OS_Lab4\\log_r") + std::to_string(i) + std::string(".txt");
        proc_handles[i] = start_r_process(logFileName);
    }
    std::cout << "Write process ID:\n";
    for (int i = 0; i < r_count; i++)
    {
        std::string logFileName = std::string("C:\\OS_Lab4\\log_w") + std::to_string(i) + std::string(".txt");
        proc_handles[w_count + i] = start_w_process(logFileName);
    }
    WaitForMultipleObjects(w_count + r_count, proc_handles, true, INFINITE);

    UnmapViewOfFile(buffer_view);
    UnmapViewOfFile(p_count_view);
    CloseHandle(buffer_mapping);
    CloseHandle(p_count_mapping);
    CloseHandle(buffer_handle);
    CloseHandle(p_count_handle);
    for (unsigned int i = 0; i < w_count + r_count; ++i)
        CloseHandle(proc_handles[i]);
    for (int i = 0; i < p_count; i++) {
        CloseHandle(free_semaphores[i]);
        CloseHandle(recorded_semaphores[i]);
    }
}
