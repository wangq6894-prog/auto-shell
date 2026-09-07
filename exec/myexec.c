#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int main()
{
    printf("我变成了一个进程: %d\n", getpid());

    pid_t id = fork();
    if(id == 0)
    {
        // 执行另一个程序的代码
        execl("/usr/bin/ls", "ls", "-a", "-l", NULL); //程序替换函数
        exit(0);
    }
    wait(NULL);
    printf("我的代码运行中....\n");
    printf("我的代码运行中....\n");
    printf("我的代码运行中....\n");
    printf("我的代码运行中....\n");
    printf("我的代码运行中....\n");
    return 0;
}
