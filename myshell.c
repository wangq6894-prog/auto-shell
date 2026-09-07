#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAXSIZE 128
#define MAXARGS 32
//shell内部维护的第一张表   命令行参数表
char *gargv[MAXARGS];
int gargc = 0;
const char* gsep = " ";
const char* GetUserName(){
    char* name = getenv("USER");
    return name;
}
const char* GetHostName(){
    char* name = getenv("HOSTNAME");
    return name;
}
const char* GetPwd(){
    char* pwd = getenv("PWD");
    return pwd;
}
void PrintCommandLine(){
    printf("[%s@%s %s]#",GetUserName(),GetHostName(),GetPwd());// 用户名@主机名 当前目录
    fflush(stdout);
}
int GetCommand(char* command_line,int size){
    if(fgets(command_line,sizeof(command_line),stdin) == NULL){
        return 0;
    }
    //2.1 去掉换行符
    command_line[strlen(command_line)-1] = '\0';

    return strlen(command_line);
}
int ParseCommand(char* command_line){
    gargc = 0;
    memset(gargv,0,sizeof(gargv)*sizeof(char*));
    gargv[0] = strtok(command_line,gsep);
    while(gargv[++gargc] = strtok(NULL,gsep));
    printf("gargc = %d\n",gargc);
    int i = 0;
    for(i = 0;i <= gargc;i++){
        printf("gargv[%d] = %s\n",i,gargv[i]);
    }
    return gargc; 
};
void ExecuteCommand(){
    pid_t id = fork();
    if(id < 0){
        return;
    }else if(id == 0){
        execvp(gargv[0],gargv);
        exit(0);
    }else{
        int status = 0;
        pid_t rid = waitpid(id,&status,0);
        if(rid > 0){
            printf("wait success,exit code :%d\n", WEXITSTATUS(status));
        }
    }
}
int main(){
    char command_line[MAXSIZE];
    while(1){
        //1.打印命令行字符串
        PrintCommandLine();
        //2.获取用户输入
        int len = GetCommand(command_line,sizeof(command_line));
        if(len == 0){
            continue;
        }
        //3.解析命令输入
        //"ls -a -l" -> "ls" "-a" "-l"
        //gargv[0] = "ls"
        //gargv[1] = "-a"
        //gargv[2] = "-l"
        //argc = 3
        ParseCommand(command_line);
        ExecuteCommand();
        sleep(1);
    }
    return 0;
}