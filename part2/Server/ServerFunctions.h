#pragma once
#include <windows.h>
#include <iostream>

bool connection = false;
HANDLE event = INVALID_HANDLE_VALUE;
HANDLE pipe = INVALID_HANDLE_VALUE;

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

void server_pipe_create(std::vector<std::string>* args) {

    if (event == INVALID_HANDLE_VALUE)
	    event = CreateEvent(nullptr, false, false, nullptr);
	if (pipe == INVALID_HANDLE_VALUE)
	    pipe = CreateNamedPipe(str2ws("\\\\.\\pipe\\labpipe").c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			256,
			256,
			0,
			nullptr);
	if (event == INVALID_HANDLE_VALUE || pipe == INVALID_HANDLE_VALUE) {
		std::cout << "Pipe or Event were not created. Error: " << GetLastError() << std::endl;
		return;
	}
	if (!connection) {
		OVERLAPPED over_pipe = OVERLAPPED();
		over_pipe.hEvent = event;
		connection = ConnectNamedPipe(pipe, &over_pipe);
		WaitForSingleObject(event, INFINITE);
		if (connection)
			std::cout << "Connected\n";
		else
			std::cout << "Connection failed: error " << GetLastError() << '\n';
	}
	else
		std::cout << "Connection already exists.\n";
}

void server_pipe_write(std::vector<std::string>* args) {
	OVERLAPPED over_write = OVERLAPPED();
	std::string message = (*args)[0];
	over_write.hEvent = event;
	if (message.size() <= 256) {
		BOOL ok = WriteFile(pipe, message.data(), message.size(), nullptr, &over_write);
		if (ok && WaitForSingleObject(event , INFINITY) == 0)
			std::cout << "Message was written sucessfully.\n";
		else
			std::cout << "Writing failed. Error: " << GetLastError() << '\n';
	}
	else
		std::cout << "Message is too big.\n";
}

void server_pipe_disconect(std::vector<std::string>* args) {
	if (DisconnectNamedPipe(pipe)) {
		connection = false;
		std::cout << "Disonnected\n";
	}
	else
		std::cout << "Disconnection failed: error " << GetLastError() << '\n';
}