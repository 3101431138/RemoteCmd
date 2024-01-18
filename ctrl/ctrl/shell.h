#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <conio.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <Windows.h>
#include <functional>

typedef std::function<void (const char* cmd_result, uint32_t length)> cmd_complete_t;

class Shell
{
public:
    Shell();
   ~Shell();
    void invoke();
    void set_cmd_complete_cb(cmd_complete_t cb);
    void invoke_commmand(std::string& command);
private:
    void internal_cmd();
private:
    std::atomic<bool>       cmd_run_;
    std::thread             cmd_thread_;
    std::mutex              cmd_mutex_;
    std::queue<std::string> cmd_queue_;
    cmd_complete_t          cmd_complete_cb_;
};