#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <list>
#include <mutex>
#include <ctime>
#include <time.h>
#include <atomic>
#include <string>
#include <memory>
#include <iostream>
#include <assert.h>
#include <stdarg.h>
#include <Shlobj.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <sys/timeb.h>
#include <condition_variable>

#pragma comment(lib, "shlwapi.lib")

#define INFO(fmt, ...) console_color_log::log(LOG_INFO, __FUNCSIG__, __LINE__, GetCurrentThreadId(), fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) console_color_log::log(LOG_WARN, __FUNCSIG__, __LINE__, GetCurrentThreadId(), fmt, ##__VA_ARGS__)
#define ERRO(fmt, ...) console_color_log::log(LOG_ERRO, __FUNCSIG__, __LINE__, GetCurrentThreadId(), fmt, ##__VA_ARGS__)
#define LOG_INIT       console_color_log::init()
#define LOG_UNINIT     console_color_log::uninit()

enum log_level
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERRO
};

class console_color_msg
{
public:
    std::string  level_;
    std::string  funcsig_;
    std::string  datetime_;
    std::string  message_;
    unsigned int line_;
    unsigned int thread_id_;
};

class console_color_log
{   
public:
    static void init();
    static void uninit();
    static void log(log_level level, const char* func_sign, int line, int thread_id, const char* fmt, ...);
private:
    void thread();
    void dispaly(console_color_msg& msg);
    void save2file(console_color_msg& msg);
    void save2xlsx(console_color_msg& msg);
    std::string time(int mode = 0);
    console_color_log(const char* log_dir = ".\\log\\", unsigned int log_max_size = (1024 * 1024 * 5));
private:
    FILE*                        log_file_;
    std::string                  log_dir_;
    std::mutex                   log_mutex_;
    unsigned int                 log_max_size_;
    std::condition_variable      log_cond_;
    std::list<console_color_msg> log_list_;
    std::unique_ptr<std::thread> log_thread_;

    static bool                  running_;
    static console_color_log*    instance_;
};

