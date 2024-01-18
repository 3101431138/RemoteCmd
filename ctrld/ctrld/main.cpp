#define _CRT_SECURE_NO_WARNINGS
#include "cmder.h"

int main()
{
    LOG_INIT;

    INFO(
        "                                                                                    \r\n"\
        "¨€¨€¨€¨€¨€¨€¨[ ¨€¨€¨€¨€¨€¨€¨€¨[¨€¨€¨€¨[   ¨€¨€¨€¨[ ¨€¨€¨€¨€¨€¨€¨[ ¨€¨€¨€¨€¨€¨€¨€¨€¨[¨€¨€¨€¨€¨€¨€¨€¨[     ¨€¨€¨€¨€¨€¨€¨[¨€¨€¨€¨[   ¨€¨€¨€¨[¨€¨€¨€¨€¨€¨€¨[ \r\n"\
        "¨€¨€¨X¨T¨T¨€¨€¨[¨€¨€¨X¨T¨T¨T¨T¨a¨€¨€¨€¨€¨[ ¨€¨€¨€¨€¨U¨€¨€¨X¨T¨T¨T¨€¨€¨[¨^¨T¨T¨€¨€¨X¨T¨T¨a¨€¨€¨X¨T¨T¨T¨T¨a    ¨€¨€¨X¨T¨T¨T¨T¨a¨€¨€¨€¨€¨[ ¨€¨€¨€¨€¨U¨€¨€¨X¨T¨T¨€¨€¨[\r\n"\
        "¨€¨€¨€¨€¨€¨€¨X¨a¨€¨€¨€¨€¨€¨[  ¨€¨€¨X¨€¨€¨€¨€¨X¨€¨€¨U¨€¨€¨U   ¨€¨€¨U   ¨€¨€¨U   ¨€¨€¨€¨€¨€¨[¨€¨€¨€¨€¨€¨[¨€¨€¨U     ¨€¨€¨X¨€¨€¨€¨€¨X¨€¨€¨U¨€¨€¨U  ¨€¨€¨U\r\n"\
        "¨€¨€¨X¨T¨T¨€¨€¨[¨€¨€¨X¨T¨T¨a  ¨€¨€¨U¨^¨€¨€¨X¨a¨€¨€¨U¨€¨€¨U   ¨€¨€¨U   ¨€¨€¨U   ¨€¨€¨X¨T¨T¨a¨^¨T¨T¨T¨T¨a¨€¨€¨U     ¨€¨€¨U¨^¨€¨€¨X¨a¨€¨€¨U¨€¨€¨U  ¨€¨€¨U\r\n"\
        "¨€¨€¨U  ¨€¨€¨U¨€¨€¨€¨€¨€¨€¨€¨[¨€¨€¨U ¨^¨T¨a ¨€¨€¨U¨^¨€¨€¨€¨€¨€¨€¨X¨a   ¨€¨€¨U   ¨€¨€¨€¨€¨€¨€¨€¨[    ¨^¨€¨€¨€¨€¨€¨€¨[¨€¨€¨U ¨^¨T¨a ¨€¨€¨U¨€¨€¨€¨€¨€¨€¨X¨a\r\n"\
        "¨^¨T¨a  ¨^¨T¨a¨^¨T¨T¨T¨T¨T¨T¨a¨^¨T¨a     ¨^¨T¨a ¨^¨T¨T¨T¨T¨T¨a    ¨^¨T¨a   ¨^¨T¨T¨T¨T¨T¨T¨a     ¨^¨T¨T¨T¨T¨T¨a¨^¨T¨a     ¨^¨T¨a¨^¨T¨T¨T¨T¨T¨a \r\n");

    int  _           = 0;
    char serv_ip[32] = { 0 };

    INFO("Input Server Ip Address: ");

    _ = scanf("%s", serv_ip);

    Cmder cmd(serv_ip);
    cmd.run();

    while(true) Sleep(100);

    LOG_UNINIT;
    return 0;
}