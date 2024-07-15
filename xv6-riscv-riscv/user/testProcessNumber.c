//系统调用实验修改10：编写测试程序
//测试系统调用功能
#include"../kernel/types.h"
#include"../kernel/stat.h"
#include"user.h"
int main(int argc,char *argv[])
{
    int count =getProcessNumber();
    printf("现在系统中有%d个进程\n",count);
    exit(0);
}