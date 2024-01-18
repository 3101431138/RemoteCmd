#include "shell.h"

Shell::Shell()
{
    cmd_run_.store(true);
}

Shell::~Shell()
{
    cmd_run_.store(false);
    if(cmd_thread_.joinable())
    {
        cmd_thread_.join();
    }
}

void Shell::invoke()
{
    cmd_thread_ = std::thread(&Shell::internal_cmd, this);
}

void Shell::set_cmd_complete_cb(cmd_complete_t cb)
{
    cmd_complete_cb_ = cb;
}

void Shell::invoke_commmand(std::string& command)
{
    std::unique_lock<std::mutex> lock(cmd_mutex_);
    cmd_queue_.push(std::move(command));
}

void Shell::internal_cmd()
{
    SECURITY_ATTRIBUTES security_attr = { 0 };
    security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attr.bInheritHandle = TRUE;
    security_attr.lpSecurityDescriptor = NULL;

    HANDLE send_request_pipe = NULL;
    HANDLE recv_request_pipe = NULL;
    HANDLE send_response_pipe = NULL;
    HANDLE recv_response_pipe = NULL;

    assert(CreatePipe(&recv_request_pipe, &send_request_pipe, &security_attr, 0));
    assert(CreatePipe(&recv_response_pipe, &send_response_pipe, &security_attr, 0));

    PROCESS_INFORMATION process_info = { 0 };
    STARTUPINFOA        startup_info = { 0 };

    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdError = send_response_pipe;
    startup_info.hStdOutput = send_response_pipe;
    startup_info.hStdInput = recv_request_pipe;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.dwFlags |= STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE;

    CHAR cmd_args[MAX_PATH] = "cmd.exe";

    assert(CreateProcessA(NULL, cmd_args, NULL, NULL, TRUE,
        0, NULL, NULL, &startup_info, &process_info));

    DWORD trans_length = 0;

    Sleep(50);

    std::string cmd_result;
    cmd_result.resize(1 * 1024 * 1024);

    while(cmd_run_.load())
    {
        Sleep(1);

        while (1)
        {
            memset((void*)cmd_result.c_str(), 0, cmd_result.size());

            DWORD bytesRead = 0;
            DWORD totalBytesAvail = 0;
            DWORD bytesLeftThisMessage = 0;

            if (!PeekNamedPipe(recv_response_pipe, (LPVOID)cmd_result.c_str(), cmd_result.size(), &bytesRead, &totalBytesAvail, &bytesLeftThisMessage) || totalBytesAvail <= 0)
            {
                break;
            }

            assert(ReadFile(recv_response_pipe, (LPVOID)cmd_result.c_str(), cmd_result.size(), &trans_length, NULL));

            if(cmd_complete_cb_) cmd_complete_cb_(cmd_result.c_str(), trans_length);
        }

        bool need_write = false;
        std::string command;
        {
            std::unique_lock<std::mutex> lock(cmd_mutex_); 
            need_write = cmd_queue_.size() > 0;
            if(need_write)
            {
                command = std::move(cmd_queue_.front());
                cmd_queue_.pop();
            }
        }

        if(need_write) assert(WriteFile(send_request_pipe, command.c_str(), command.length(), &trans_length, NULL));
    }

    CloseHandle(send_request_pipe);
    CloseHandle(recv_request_pipe);
    CloseHandle(send_response_pipe);
    CloseHandle(recv_response_pipe);

    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
}
