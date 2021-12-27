#include <iostream>
#include <windows.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <string>

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

int AddLog(std::string str, HANDLE outHandle)
{
	DWORD writtenBytes;
	WriteFile(outHandle, str.data(), str.size(), &writtenBytes, nullptr);
	return writtenBytes;
}

long long GetTime(std::chrono::steady_clock::time_point start) {
	auto point = std::chrono::high_resolution_clock::now();
	return (std::chrono::duration_cast<std::chrono::milliseconds>(point - start)).count();;
}

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	char str_id[16] = "               ";
	unsigned int id = GetCurrentProcessId();
	std::string row_str_id = std::to_string(id);
	for (int i = 0; i < row_str_id.size(); ++i)
		str_id[i] = row_str_id[i];

	HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE p_count_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, str2ws("Pages_count_mapping").c_str());
	HANDLE p_count_view = MapViewOfFile(p_count_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 12);

	unsigned int p_count;
	unsigned int op_count;
	memcpy(&p_count, p_count_view, 4);
	memcpy(&op_count, (char*)p_count_view+8, 4);

	
	HANDLE* free_semaphores = new HANDLE[p_count];
	HANDLE* recorded_semaphores = new HANDLE[p_count];
	for (int i = 0; i < p_count; ++i) {
		free_semaphores[i] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, str2ws("freeSemaphore" + std::to_string(i)).c_str());
		recorded_semaphores[i] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, str2ws("recordedSemaphore" + std::to_string(i)).c_str());
	}

	HANDLE buffer_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, str2ws("Buffer_mapping").c_str());
	LPVOID buffer_view = MapViewOfFile(buffer_mapping, FILE_MAP_ALL_ACCESS, 0, 0, p_count * 4096);
	size_t page;


	if (buffer_view != INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < op_count; i++)
		{
			//Start of waitig
			page = WaitForMultipleObjects(p_count, free_semaphores, false, INFINITE);
			AddLog(std::to_string(GetTime(start)) + std::string(" | TAKE SEMAPHORE\n"), std_out);
			
			memcpy((void*)((int)buffer_view + page * 4096), str_id, 16);
			SleepEx(rand()%1000 + 500, false);

			if (ReleaseSemaphore(recorded_semaphores[page], 1, nullptr)) {
				AddLog(std::to_string(GetTime(start)) + std::string(" | FREE SEMAPHORE\n"), std_out);
				AddLog(std::to_string(GetTime(start)) + std::string(" | PAGE ") + std::to_string(page) + '\n' + '\n', std_out);
			}
			else
				AddLog(std::to_string(GetTime(start)) + std::string(" | SEMAPHORE ERROR:") + std::to_string(GetLastError()) + '\n' + '\n', std_out);
		}
	}
	else
	{
		AddLog(std::to_string(GetTime(start)) + std::string(" | SEMAPHORE INIT ERROR:") + std::to_string(GetLastError()) + '\n', std_out);
	}

	CloseHandle(std_out);
}

