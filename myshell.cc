#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#define MAXSIZE 128
#define MAXARGS 32
//shell内部维护的第一张表   命令行参数表
char *gargv[MAXARGS];
int gargc = 0;
const char* gsep = " ";
//环境变量表
char* genv[MAXARGS];
int genvc = 0;
// 最近一个命令执行完毕，退出码
int lastcode = 0;

#define NoneRedir 0
#define InputRedir 1
#define AppRedir 2
#define OutputRedir 3

int redir_type = NoneRedir;
char* filename = NULL;

#define TrimSpace(start) do{while(isspace(*start)) start++;}while(0)
void LoadEnv()
{
    // 正常情况，环境变量表内部是从配置文件来的
    // 这里从父进程拷贝环境变量表
    extern char **environ;
    for(; environ[genvc]; genvc++)
    {
        genv[genvc] = (char*)malloc(sizeof(char)*4096);
        strcpy(genv[genvc], environ[genvc]);
    }
    genv[genvc] = NULL;

    printf("Load env: \n");
    for(int i = 0; genv[i]; i++)
        printf("genv[%d]: %s\n", i, genv[i]);
}
static std::string rfindDir(const std::string &p)
{
    if(p == "/")
        return p;
    const std::string psep = "/";
    auto pos = p.rfind(psep);
    if(pos == std::string::npos)
        return std::string();
    return p.substr(pos+1); // /home/whb
}
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
    if(fgets(command_line,size,stdin) == NULL){
        return 0;
    }
    //2.1 去掉换行符
    command_line[strlen(command_line)-1] = '\0';

    return strlen(command_line);
}
void  ParseRedir(char commandline[]){
    redir_type = NoneRedir;
    filename = NULL;
    char* start = commandline;
    char* end = commandline + strlen(commandline);
    while(start < end){
        if(*start == '<'){
            //输入重定向
            *start = '\0';
            start++;
            TrimSpace(start);//去掉/跳过空格
            redir_type = InputRedir;
            filename = start;
            break;
        }else if(*start == '>'){
            if(*(start + 1) == '>'){
                //追加重定向
                *start = '\0';
                start++;
                *start = '\0';
                start++;
                TrimSpace(start);//去掉/跳过空格
                redir_type = AppRedir;
                filename = start;
                break;
            }
            //输出重定向
            *start = '\0';
            start++;
            TrimSpace(start);//去掉/跳过空格
            redir_type = OutputRedir;
            filename = start;
            break;
        }else{
            //暂时没有重定向
            start++;
        }
    }
    return start;
}
int ParseCommand(char* command_line){
    gargc = 0;
    memset(gargv,0,sizeof(gargv)*sizeof(char*));
    gargv[0] = strtok(command_line,gsep);
    while(gargv[++gargc] = strtok(NULL,gsep));
    printf("gargc = %d\n",gargc);
    int i = 0;
    // for(i = 0;i <= gargc;i++){
    //     printf("gargv[%d] = %s\n",i,gargv[i]);
    // }
    return gargc; 
};
// retunr val:
// 0 : 不是内建命令
// 1 : 内建命令&&执行完毕
int CheckBuiltinExecute()
{   //查询是否是内建命令
    if(strcmp(gargv[0], "cd") == 0)
    {
        // 内建命令
        if(gargc == 2)
        {
            // 新的目标路径: gargv[1]
            // 1. 更改进程内核中的路径
            chdir(gargv[1]);
            // 2. 更改环境变量
            char pwd[1024];
            getcwd(pwd, sizeof(pwd)); // /home/whb
            static char cwd[1024]; // putenv 长期持有指针，须为静态
            snprintf(cwd, sizeof(cwd), "PWD=%s", pwd); // cwd: PWD=/home/home
            putenv(cwd);
            lastcode = 0;
        }
        return 1;
    }
    else if(strcmp(gargv[0], "echo") == 0) // cd , echo , env , export 内建命令
    {
        if(gargc == 2)
        {
            if(gargv[1][0] == '$')
            {
                // $? ? : 看做一个变量名字
                if(strcmp(gargv[1]+1, "?") == 0)
                {
                    printf("lastcode: %d\n", lastcode);
                }
                else if(strcmp(gargv[1]+1, "PATH") == 0)
                {
                    // 不准你用getenv和putenv
                    printf("%s\n", getenv("PATH")); // putenv 和 getenv 究竟是什么, 访问环境变量表！
                }
                lastcode = 0;
            }
            return 1;
            // echo helloworld
            // echo $?
        }
    }
    return 0;
}
void ExecuteCommand(){
    pid_t id = fork();
    if(id < 0){
        return;
    }else if(id == 0){
        int fd = -1;
        if(redir_type == NoneRedir)
        {
            // Do Nothing
        }
        else if(redir_type == OutputRedir)
        {
            // 子进程要进行输出重定向
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd, 1);
        }
        else if(redir_type == AppRedir)
        {
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
            dup2(fd, 1);
        }
        else if(redir_type == InputRedir)
        {
            fd = open(filename, O_RDONLY);
            dup2(fd, 0);
        }
        else{
            //bug??
        }
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
        ParseRedir(command_line);
        ParseCommand(command_line);
        //4.查询是否是内建命令
        if(CheckBuiltinExecute() == 1){
            continue;
        }
        ExecuteCommand();
    }
    return 0;
}