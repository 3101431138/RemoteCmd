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
#include "console_color_log.h"

typedef std::function<void (const char* cmd_result, uint32_t length)> cmd_complete_t;

class Shell
{
public:
    Shell();
   ~Shell();
    void invoke();                              /*启动          */
    void set_cmd_complete_cb(cmd_complete_t cb);/*设置运行结果回调*/
    void invoke_commmand(std::string& command); /*执行一条命令   */
private:
    void internal_cmd();
private:
    std::atomic<bool>       cmd_run_;           /*运行状态   */
    std::thread             cmd_thread_;        /*处理线程   */
    std::mutex              cmd_mutex_;         /*保护命令队列*/
    std::queue<std::string> cmd_queue_;         /*命令队列   */
    cmd_complete_t          cmd_complete_cb_;   /*运行结果回调*/
};