#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERSIZE 200
int main(int argc,char *argv[]){
    if(argc==1){
        printf("Help command is give information list of commands.\n");
    }
    if(argc==2){
        if(strcmp(argv[1],"ls")==0){
            printf("This command lists file type, access rights, file sizes and file name of all files in the present working directory \n");
        }
        if(strcmp(argv[1],"pwd")==0){
            printf("This command prints the present working directory \n");
        }
        if(strcmp(argv[1],"cd")==0){
            printf("This command which will change the present working directory to the location provided as argument\n");
        }
        if(strcmp(argv[1],"cat")==0){
            printf("This command  print on standard output the contents of the file provided to it as argument \n");
        }
        if(strcmp(argv[1],"wc")==0){
            printf("this command  print on standard output the number of lines in the file provided to it as argument \n");
        }
        if(strcmp(argv[1],"exit")==0){
            printf("this command  exits the shell. \n");
        }
    }
}
