#include <cstdio>
#include <exception>
#include <windows.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <locale>
#include <iostream>

#include "usb.hpp"

#define uchar unsigned char
#define uint unsigned int
const char* path = "./POTN.bin";

uchar buffer[128 * 1024];   //需要升级的二进制文件
uint len = 0;               //

void init();
void cleanAndExit(int sig);
bool informUpgrepStart(USB::Connector& usb);
void controlLoop(USB::Connector& usb, USB::Filer& file);

int main(void)
{     
    std::cout << "Hello World!\n";


    while (1);
    return 0;
}

void init()
{
    signal(SIGABRT, cleanAndExit);  // 异常终止 abort
    signal(SIGTERM, cleanAndExit);  //终止
    signal(SIGINT, cleanAndExit);   //CTRL+C Interrupt from keyboard
    signal(SIGSEGV, cleanAndExit);  //访问无效内存 或没有权限的内存 
    signal(SIGILL, cleanAndExit);   //指令中有非法指令
    signal(SIGFPE, cleanAndExit);   // 浮点运算错误

    setlocale(LC_ALL, "chs");      //地域设置，编码ANSI 中文不会乱码

}

void cleanAndExit(int sig)
{
    system("pause");       // DOS调用 黑框框不会闪退 
    exit(sig);
}


void controlLoop(USB::Connector& usb,USB::Filer& file) 
{
    while (true) 
    {
        if (!usb.Connect())
        {
            puts("Waiting for device...");
            Sleep(1000);
            continue;
        }
        if(!informUpgrepStart(usb))
        {
            puts("device be missing...");
            Sleep(1000);
            continue;
        }
    }

}

bool informUpgrepStart(USB::Connector& usb)
{
    BYTE Receive[USB::Connector::MAX_PACK_SIZE];
    BYTE Send[USB::Connector::MAX_PACK_SIZE];
    memset(Send,0,sizeof(Send));
    Send[0] = USB::Connector::UPGRED_PACK;
    while(true)
    {
        Sleep(1000);       
        int actualLen = 0;
        USB::Error err = usb.Read(Receive,sizeof(Receive),&actualLen);
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }
        if(USB::Error::OK == err)
        {
            if (usb.UPGRED_READY_PACK == Receive[0])
            {
                return true;
            }
            else
            {
                continue;
            }
        }

        err = usb.Write(Send, sizeof(Send));
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }
        if (USB::Error::OK != err)
        {
            continue;
        }
    }
}

bool UpgrepData(USB::Connector& usb, USB::Filer& file)
{
    USB::Error err;
    BYTE Receive[USB::Connector::MAX_PACK_SIZE];
    BYTE Send[USB::Connector::MAX_PACK_SIZE];
    int actualLen = 0;
    Send[0] = usb.UPGRED_DATA_PACK;

    while (true)
    {
        memcpy(file.Buffer + file.Index, Send + 1, sizeof(Send) - 1);
        file.Index += (sizeof(Send) - 1);
        err = usb.Write(Send, sizeof(Send));
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }

        err = usb.Read(Receive, sizeof(Receive), &actualLen);
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }
        if (USB::Error::TIMEOUT == err)     
        {
            file.Index -= (sizeof(Send) - 1);  //超时说明上一包没有收到 所以重新发送
            continue;
        }

    }


}





// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

