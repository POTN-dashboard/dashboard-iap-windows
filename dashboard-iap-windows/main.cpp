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
uchar buffer[128 * 1024];   
uint len = 0;

void init();
void cleanAndExit(int sig);
void gain_bin(const char* path);

int main(void)
{     
    std::cout << "Hello World!\n";
    gain_bin(path);
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

void gain_bin(const char *path)
{
    FILE* fp = NULL;
    
    uchar checksum = 0;
    errno_t err = 0;

    if((err = fopen_s(&fp,path,"rb")) != 0){      //二进制可读可写打开
        std::cout << "文件打开失败" << std::endl;
        system("pause");       // DOS调用 黑框框不会闪退 
        exit(0);
    }
    if (fp != 0) {                  //文件打开成功
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);    //获取文件大小
        rewind(fp);		//重新恢复位置指针的位置，回到文件的开头
        printf("本次升级的bin文件一共有%d个字节\n", len);
        fread(&buffer,1,len,fp);

        //for(uint i = 0; i < len; i++){
        //    checksum += buffer[i];
        //}
        fclose(fp);
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

