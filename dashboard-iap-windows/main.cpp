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


void init();
void cleanAndExit(int sig);
bool informUpgrepStart(USB::Connector& usb);
bool upgrepInform(USB::Connector& usb, USB::Filer& file);
bool upgrepData(USB::Connector& usb, USB::Filer& file);
bool upgrepStatus(USB::Connector& usb);
void controlLoop(USB::Connector& usb, USB::Filer& file);

USB::Filer file;
int main(void)
{  
    try
    {
        init();
        USB::Connector usb;
        controlLoop(usb, file); 
    }
    catch (const std::exception & e)     //捕获异常
    {
        puts(e.what());
    }

    cleanAndExit(0);
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
        puts("Device connected!");

        if(!informUpgrepStart(usb))
        {
            puts("Device be missing...");
            Sleep(1000);
            continue;
        }
        puts("Receive STM32 ready upgrep pack");

        if (!upgrepInform(usb,file))
        {
            puts("Send inform failed...");
            Sleep(1000);
            continue;
        }
        puts("Send information pack");

        if (!upgrepData(usb,file))
        {
            puts("send data failed...");
            Sleep(1000);
            continue;
        }
        puts("Send all data pack");

        if(!upgrepStatus(usb))
        {
            puts("checksum is error");
            Sleep(1000);
            continue;
        }
        puts("checksum is OK");
        return ;
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
        memset(Receive, 0, sizeof(Receive));
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
        }

        err = usb.Write(Send, sizeof(Send));
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }

    }
}

bool upgrepInform(USB::Connector& usb, USB::Filer& file)
{
    BYTE Receive[USB::Connector::MAX_PACK_SIZE];
    BYTE Send[USB::Connector::MAX_PACK_SIZE];
    memset(Send,0,sizeof(Send));

    Send[0] = usb.UPGRED_INFORM_PACK;
    if (true == file.getBin(path)) {
        Send[1] = (INT8)(file.Size >> 8*3);
        Send[2] = (INT8)(file.Size << 8 >> 8 * 3);
        Send[3] = (INT8)(file.Size << 8*2 >> 8 * 3);
        Send[4] = (INT8)(file.Size << 8 * 3 >> 8 * 3);
        Send[5] = file.Checksum;
        while(true)
        {
            Sleep(1000);
            memset(Receive, 0, sizeof(Receive));
            USB::Error err = usb.Write(Send,sizeof(Send));
            if (USB::Error::DISCONNECTED == err)
            {
                return false;
            }
            if (USB::Error::OK != err)
            {
                continue;
            }

            int actualLen = 0;
            err = usb.Read(Receive,sizeof(Receive),&actualLen);
            if (USB::Error::DISCONNECTED == err)
            {
                return false;
            }
            if (USB::Error::OK != err)
            {
                continue;
            }
            if (usb.ACK_PACK == Receive[0])
            {
                return true;
            }       
        }
    }

}


bool upgrepData(USB::Connector& usb, USB::Filer& file)
{
    USB::Error err;
    BYTE Receive[USB::Connector::MAX_PACK_SIZE];
    BYTE Send[USB::Connector::MAX_PACK_SIZE];
    memset(Send, 0, sizeof(Send));
    int actualLen = 0;
    Send[0] = usb.UPGRED_DATA_PACK;

    while (file.Index/(usb.MAX_PACK_SIZE - 3) < file.Size/(usb.MAX_PACK_SIZE - 3))  //发送61字节全部被沾满的包 
    {
        Send[1] = (INT8)(file.PackNum >> 8);
        Send[2] = (INT8)(file.PackNum << 8 >> 8);
        memset(Receive, 0, sizeof(Receive));
        memcpy(Send + 3,file.Buffer + file.Index,sizeof(Send) - 3);
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
            continue;
        }
        
        if (usb.ACK_PACK == Receive[0])        //发送成功，包的编号+1，buffer索引+61
        {
        printf("Send num %d is ok\n", file.PackNum);
        file.PackNum++;
        file.Index += (sizeof(Send) - 3);
        }
    }

    if(file.Size%(usb.MAX_PACK_SIZE - 3) == 0)      //如果总字节数是 61 的整数倍 发送结束
    {
        return true;
    }

    while (true)                          //发送余下的 61余数个字节的数据
    {
        Send[1] = (INT8)(file.PackNum >> 8);
        Send[2] = (INT8)(file.PackNum << 8 >> 8);
        memset(Receive, 0, sizeof(Receive));
        memcpy(Send + 3, file.Buffer + file.Index, (file.Size % (usb.MAX_PACK_SIZE - 3)));
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
            continue;
        }
        if (usb.ACK_PACK == Receive[0])          //发送成功，包的编号+1，buffer索引+61
        {
            printf("Send num %d is ok\n", file.PackNum);
            return true;
        }

    }

}


bool upgrepStatus(USB::Connector& usb)
{
    BYTE Receive[USB::Connector::MAX_PACK_SIZE];
    USB::Error err;
    memset(Receive,0,sizeof(Receive));
    int actualLen = 0;
    while (true)
    {
        err = usb.Read(Receive, sizeof(Receive), &actualLen);
        if (USB::Error::DISCONNECTED == err)
        {
            return false;
        }
        if (USB::Error::TIMEOUT == err)
        {
            continue;
        }
        if (usb.UPGRED_STATUS_PACK == Receive[0])
        {
            if (0 == Receive[1])        // 0 为升级成功
            {
                return true;
            } 
            return false;     
            
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

