#pragma once
#include <windows.h>
#include <iostream>

bool connection = false;
HANDLE event = INVALID_HANDLE_VALUE;
HANDLE pipe = INVALID_HANDLE_VALUE;

void WINAPI callback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) { std::cout << "Message is received\n";}

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

void client_pipe_create(std::vector<std::string>* args) {

	if (!connection) {
		pipe = CreateFile(str2ws("\\\\.\\pipe\\labpipe").c_str(), GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (pipe != INVALID_HANDLE_VALUE) {
			connection = true;
			std::cout << "Connected.\n";
		}
		else
			std::cout << "Connection failed. Error: " << GetLastError() << '\n';
	}
	else
		std::cout << "Connection already exists.\n";
}

void client_pipe_write(std::vector<std::string>* args) {
	OVERLAPPED over_read = OVERLAPPED();
	char message[256] = {'\0'};
	if (ReadFileEx(pipe, message, 256, &over_read, callback)) {
		SleepEx(INFINITE, true);
		std::cout << "Message: " << message << '\n';
	}
	else
		std::cout << "Reading failed: error " << GetLastError() << '\n';
}

void client_pipe_disconect(std::vector<std::string>* args) {
	if (CloseHandle(pipe)) {
		connection = false;
		pipe = INVALID_HANDLE_VALUE;
		std::cout << "Disonnected\n";
	}
	else
		std::cout << "Disconnection failed: error " << GetLastError() << '\n';
}