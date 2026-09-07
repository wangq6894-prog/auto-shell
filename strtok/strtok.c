#include <stdio.h>
#include <string.h>
int main(){
    char str[] = "aaaa bbbb cccc dddd";
    const char* sep = " #:";
    char* p1 = strtok(str,sep);
    printf("%s\n",p1);
    char* p2 = strtok(NULL,sep);
    printf("%s\n",p2);
}