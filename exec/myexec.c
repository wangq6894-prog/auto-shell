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
        sleep(2);
        printf("子进程运行中....\n");
        // 执行另一个程序的代码
        //execl("/usr/bin/ls", "ls", "-a", "-l", NULL); //程序替换函数
        //execlp("ls","ls","-a","-l",NULL);
        char* argv[] = {"ls","-a","-l",NULL};
        execv("/usr/bin/ls",argv);
        exit(1);
    }
    //wait(NULL);
    int status = 0;
    pid_t rid = waitpid(id, &status, 0);
    if(rid >0){
        printf("wait success,exit code :%d\n", WEXITSTATUS(status));
    }
    return 0;
}
