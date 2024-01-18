#include "shell.h"

Shell::Shell()
{
    cmd_run_.store(true);
}

Shell::~Shell()
{
    //等待线程结束
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
    //存放一条将要执行的命令
    std::unique_lock<std::mutex> lock(cmd_mutex_);
    cmd_queue_.push(std::move(command));
}

void Shell::internal_cmd()
{
    SECURITY_ATTRIBUTES security_attr  = { 0 };
    security_attr.nLength              = sizeof(SECURITY_ATTRIBUTES);   /*SECURITY_ATTRIBUTES结构大小*/
    security_attr.bInheritHandle       = TRUE;                          /*子进程是否可以继承句柄       */
    security_attr.lpSecurityDescriptor = NULL;                          /*安全描述符,一般不用         */

    HANDLE send_request_pipe  = NULL;   /*向子进程发送命令的管道          */
    HANDLE recv_request_pipe  = NULL;   /*子进程从父进程接收命令的管道     */
    HANDLE send_response_pipe = NULL;   /*子进程向父进程发送命令结果的管道  */
    HANDLE recv_response_pipe = NULL;   /*从子进程中接收命令结果的管道     */

    /*创建收发命令的管道*/
    assert(CreatePipe(  &recv_request_pipe, /*接收数据的管道*/
                        &send_request_pipe, /*发送数据的管道*/
                        &security_attr,     /*安全属性     */
                        0  ));              /*缓冲区大小   */

    /*创建收发命令结果的管道*/
    assert(CreatePipe(  &recv_response_pipe,
                        &send_response_pipe, 
                        &security_attr, 
                        0  ));

    PROCESS_INFORMATION process_info = { 0 };
    STARTUPINFOA        startup_info = { 0 };

    startup_info.cb           = sizeof(STARTUPINFOA);   /*STARTUPINFOA结构大小                        */
    startup_info.hStdError    = send_response_pipe;     /*子进程标准错误输出句柄                        */
    startup_info.hStdOutput   = send_response_pipe;     /*子进程标准普通输出句柄                        */
    startup_info.hStdInput    = recv_request_pipe;      /*子进程标准普通输入句柄                        */
    startup_info.dwFlags     |= STARTF_USESTDHANDLES;   /*表示hStdError, hStdOutput, hStdInput包含信息 */
    startup_info.dwFlags     |= STARTF_USESHOWWINDOW;   /*表示wShowWindow包含信息                      */
    startup_info.wShowWindow  = SW_HIDE;                /*隐藏窗口                                     */

    CHAR cmd_args[MAX_PATH] = "cmd.exe";

    //创建Cmd.exe进程
    assert(CreateProcessA(  NULL,               /*Exe路径               */
                            cmd_args,           /*命令行参数             */
                            NULL,               /*进程安全属性           */
                            NULL,               /*线程安全属性           */
                            TRUE,               /*子进程是否继承父进程句柄 */
                            0,                  /*进程创建标志           */
                            NULL,               /*环境变量               */
                            NULL,               /*进程当前目录            */
                            &startup_info,      /*启动信息               */
                            &process_info  ));  /*进程信息               */

    DWORD trans_length = 0;

    //运行结果缓冲区
    std::string cmd_result;
    cmd_result.resize(1 * 1024 * 1024);

    while(cmd_run_.load())
    {
        //限制效率
        Sleep(1);

        //读取完所有命令结果
        while (1)
        {
            memset((void*)cmd_result.c_str(), 0, cmd_result.size());

            DWORD bytesRead = 0;
            DWORD totalBytesAvail = 0;
            DWORD bytesLeftThisMessage = 0;

            //检查是否有命令结果
            if (!PeekNamedPipe(
                recv_response_pipe,     /*管道句柄    */
                NULL,                   /*缓冲区      */
                NULL,                   /*缓冲区大小   */
                &bytesRead,             /*读取到多少数据*/
                &totalBytesAvail,       /*有多少数据可读*/
                &bytesLeftThisMessage)  /*还剩多少数据  */
                || totalBytesAvail <= 0)
            {
                break;
            }
    
            //读取命令结果
            assert(ReadFile(recv_response_pipe, /*管道句柄      */
                (LPVOID)cmd_result.c_str(),     /*缓冲区        */ 
                cmd_result.size(),              /*最多想读多少数据*/
                &trans_length,                  /*读取了多少数据  */
                NULL));                         /*重叠结构       */

            INFO(cmd_result.c_str());

            //调用回调
            if(cmd_complete_cb_) cmd_complete_cb_(cmd_result.c_str(), trans_length);
        }

        //取一个命令
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

        //发给Cmd进程
        if(need_write) 
        {
            INFO(command.c_str());
            assert( WriteFile(send_request_pipe,/*管道句柄       */
                    command.c_str(),            /*缓冲区         */
                    command.length(),           /*最多想写多少数据*/
                    &trans_length,              /*写入了多少数据  */
                    NULL ));                    /*重叠结构       */
        }
    }

    //关闭管道句柄
    CloseHandle(send_request_pipe);
    CloseHandle(recv_request_pipe);
    CloseHandle(send_response_pipe);
    CloseHandle(recv_response_pipe);
    //关闭进程/线程句柄
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
}
