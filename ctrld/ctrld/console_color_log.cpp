#include "console_color_log.h"

#pragma warning(disable: 26812)

bool               console_color_log::running_  = false;
console_color_log* console_color_log::instance_ = nullptr;

console_color_log::console_color_log(const char* log_dir/* = ".\\log\\"*/, unsigned int log_max_size/* = (1024 * 1024 * 5)*/)
    : log_file_(nullptr)
    , log_max_size_(log_max_size)
    , log_dir_(log_dir)
{
    if (log_dir[0] == '.')
    {
        std::string exe_path;
        exe_path.resize(MAX_PATH);
        if (GetModuleFileNameA(NULL, (char*)exe_path.c_str(), MAX_PATH) != 0)
        {
            int split = exe_path.rfind("\\");
            exe_path.resize(split);
        }
        exe_path.append(&log_dir[1]);
        if (!PathFileExistsA(exe_path.c_str()))
            assert(SHCreateDirectoryExA(NULL, exe_path.c_str(), NULL) == ERROR_SUCCESS);
        log_dir_ = std::move(exe_path);
    }
    else
    {
        if (!PathFileExistsA(log_dir))
            assert(SHCreateDirectoryExA(NULL, log_dir, NULL) == ERROR_SUCCESS);
    }
}

void console_color_log::log(log_level level, const char* func_sign, int line, int thread_id, const char* fmt, ...)
{
    if (!running_)
    {
        return;
    }

    console_color_log* _this = instance_;

    va_list varg = { 0 };

    size_t info_len = 0;
    const char* log_level = "";

    switch (level)
    {
    case LOG_INFO:
        log_level = "INFO";
        break;
    case LOG_WARN:
        log_level = "WARN";
        break;
    case LOG_ERRO:
        log_level = "ERRO";
        break;
    }

    va_start(varg, fmt);

    info_len = vsnprintf(nullptr, 0, fmt, varg) + 3;

    std::string info;
    info.resize(info_len);

    vsprintf((char*)info.c_str(), fmt, varg);

    strcpy((char*)info.c_str() + info_len - 3, "\r\n");

    va_end(varg);

    std::unique_lock<std::mutex> lock(_this->log_mutex_);
    _this->log_list_.emplace_back();
    _this->log_list_.back().level_     = log_level;
    _this->log_list_.back().funcsig_   = func_sign;
    _this->log_list_.back().datetime_  = _this->time(1);
    _this->log_list_.back().message_   = info;
    _this->log_list_.back().line_      = line;
    _this->log_list_.back().thread_id_ = thread_id;
    _this->log_cond_.notify_one();
}

void console_color_log::init()
{
    if (running_)
    {
        return;
    }

    if (instance_)
    {
        return;
    }

    running_ = true;

    assert(instance_ = new console_color_log);

    console_color_log* _this = instance_;
    
    _this->log_thread_.reset(new std::thread(&console_color_log::thread, _this));
}

void console_color_log::uninit()
{
    if (!running_)
    {
        return;
    }

    if (!instance_)
    {
        return;
    }

    running_ = false;

    console_color_log* _this = instance_;

    _this->log_cond_.notify_one();
    _this->log_thread_->join();

    if (_this->log_file_) 
        fclose(_this->log_file_);

    delete instance_;
}

void console_color_log::thread()
{
    while (running_)
    {
        console_color_msg msg;
        {
            std::unique_lock<std::mutex> lock(log_mutex_);
            log_cond_.wait(lock, [this]()
            {
                if (!log_list_.empty() || running_ == false)
                    return true;
                else
                    return false;
            });

            if(running_ == false)
                return;

            msg = std::move(log_list_.front());
            log_list_.pop_front();
        }
        dispaly(msg);
        save2file(msg);
        save2xlsx(msg);
    }
}

void console_color_log::dispaly(console_color_msg& msg)
{
    static const char* color = "\033[0m";
    static const char* clear = "\033[0m";
    if (!strncmp(msg.level_.c_str(), "INFO", 4))
    {
        color = "\033[1;32m";
    }
    else if (!strncmp(msg.level_.c_str(), "WARN", 4))
    {
        color = "\033[1;33m";
    }
    else if (!strncmp(msg.level_.c_str(), "ERRO", 4))
    {
        color = "\033[1;31m";
    }
    printf("%s[%08X] %s (%s:%06d) %s <=> %s%s",
        color,
        msg.thread_id_,
        msg.datetime_.c_str(),
        msg.funcsig_.c_str(),
        msg.line_,
        msg.level_.c_str(),
        msg.message_.c_str(),
        clear);
}

void console_color_log::save2file(console_color_msg& msg)
{
    if (log_file_ == nullptr)
    {
        std::string file_name = log_dir_ + time(0) + ".log";
        log_file_ = fopen(file_name.c_str(), "wb");
        assert(log_file_);
    }
    if ((unsigned int)ftell(log_file_) >= log_max_size_)
    {
        fclose(log_file_);
        std::string file_name = log_dir_ + time(0) + ".log";
        log_file_ = fopen(file_name.c_str(), "wb");
        assert(log_file_);
    }

    std::string info;
    info.resize(128 + msg.funcsig_.length() + msg.message_.length());

    sprintf((char*)info.c_str() ,"[%08X] %s (%s:%06d) %s <=> %s",
        msg.thread_id_,
        msg.datetime_.c_str(),
        msg.funcsig_.c_str(),
        msg.line_,
        msg.level_.c_str(),
        msg.message_.c_str());

    size_t info_len = strlen(info.c_str());
    size_t writ_len = 0;
    size_t writ_ret = 0;
    do 
    {
        writ_ret = fwrite(info.c_str() + writ_len, 1, info_len - writ_len, log_file_);
        assert(writ_ret > 0);
        writ_len += writ_ret;
        fflush(log_file_);
    } while (writ_len < info_len);
}

void console_color_log::save2xlsx(console_color_msg& msg)
{
}

std::string console_color_log::time(int mode/* = 0*/)
{
    std::string time_str;
    time_str.resize(128);

    struct timeb tp;
    ftime(&tp);

    time_t now = tp.time;
    tm time;

    localtime_s(&time, &now);

    if(mode == 0)
        snprintf((char*)time_str.c_str(), 128, "%04d-%02d-%02d %02d-%02d-%02d-%03d",
            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, tp.millitm);
    else
        snprintf((char*)time_str.c_str(), 128, "%04d/%02d/%02d-%02d:%02d:%02d:%03d",
            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, tp.millitm);

    time_str.resize(strlen(time_str.c_str()));

    return time_str;
}
