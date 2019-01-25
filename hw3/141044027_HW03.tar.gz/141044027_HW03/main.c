#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#define DELIMETERS " \t\r\n\a"

int myexecV(char *command,char *argument){
    char *arguments[] = {"", argument, NULL};
    if(strcmp(command,"ls")==0){
        arguments[0]=command;
        execvp("./ls", arguments);
    }
    if(strcmp(command,"pwd")==0){
        arguments[0]=command;
        execvp("./pwd", arguments);
    }
    if(strcmp(command,"cd")==0){
        arguments[0]=command;
        arguments[1]=argument;
        execvp("./cd", arguments);
    }
    if(strcmp(command,"help")==0){
        arguments[0]=command;
        arguments[1]=argument;
        execvp("./help", arguments);
    }
    if(strcmp(command,"cat")==0){
        arguments[0]=command;
        arguments[1]=argument;
        execvp("./cat", arguments);
    }
    if(strcmp(command,"wc")==0){
        arguments[0]=command;
        arguments[1]=argument;
        execvp("./wc", arguments);
    }
}

int launch(char **args, int count)
{
    printf("count: %d\n",count);

    if(strcmp(args[0],"exit")==0)
        return 0;

    if(count<=2){
        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                //////islemin basladgı yer
                myexecV(args[0],args[1]);
                /////islemın bttgi yer
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        for(int i=0; i<count; i++){
            wait(NULL);
        }
    }
    if(count==3){
        if(strcmp(args[0],"cat")==0){
            myexecV(args[0],args[2]);
            return 1;
        }

        ///////////////PROCESS 1
        int pfd1[2];
        if(pipe(pfd1)==-1)
            perror("pipe");

        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                if(close(pfd1[0])==-1){}
                if(pfd1[1]!=STDOUT_FILENO){
                    if(dup2(pfd1[1],STDOUT_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd1[1])==-1)
                        perror("close 2");
                }
                //////islemin basladgı yer
                myexecV(args[0],NULL);
                /////islemın bttgi yer
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        ////////////////PROCESS 1 END
        ///////////////PROCESS 2
        int pfd2[2];
        if(pipe(pfd2)==-1)
            perror("pipe");
        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                if(close(pfd1[1])==-1){}
                if(pfd1[0]!=STDIN_FILENO){
                    if(dup2(pfd1[0],STDIN_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd1[0])==-1)
                        perror("close 2");
                }

                /////////////////islemin basladgı yer
                myexecV(args[2],NULL);
                /////////////////islemin bttigi yer

                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        if(close(pfd1[0])== -1)
            perror("close");
        if(close(pfd1[1])== -1)
            perror("close");
        if(close(pfd2[0])== -1)
            perror("close");
        if(close(pfd2[1])== -1)
            perror("close");

        for(int i=0; i<count; i++){
            wait(NULL);
        }
    }

    if(count==5){
        ///////////////PROCESS 1
        int pfd1[2];
        if(pipe(pfd1)==-1)
            perror("pipe");
        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                if(close(pfd1[0])==-1){}
                if(pfd1[1]!=STDOUT_FILENO){
                    if(dup2(pfd1[1],STDOUT_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd1[1])==-1)
                        perror("close 2");
                }

                //////islemin basladgı yer
                //printf("burdanbasladı1");
                myexecV(args[0],NULL);
                /////islemın bttgi yer
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        ////////////////PROCESS 1 END
        ///////////////PROCESS 2
        int pfd2[2];
        if(pipe(pfd2)==-1)
            perror("pipe");
        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                if(close(pfd1[1])==-1){}
                if(pfd1[0]!=STDIN_FILENO){
                    if(dup2(pfd1[0],STDIN_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd1[0])==-1)
                        perror("close 2");
                }

                ////islemin basladgı yer
                //char buffer[15];
                //fgets(buffer, 15 , stdin);
                myexecV(args[2],NULL);
                ////islemın bttgi yer

                //inputu outputa verme
                if(close(pfd2[0])==-1){}
                if(pfd2[1]!=STDOUT_FILENO){
                    if(dup2(pfd2[1],STDOUT_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd2[1])==-1)
                        perror("close 2");
                }
                /////////////////islemin basladgı yer
                //printf("aldim(PROCESS2) :%s",buffer);
                /////////////////islemin bttigi yer
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        ///////////PROCESS 2 END
        ///////////pROCESS 3
        switch(fork()){
            case -1:
                perror("fork");
                break;
            case 0:
                if(close(pfd2[1])==-1){}
                if(pfd2[0]!=STDIN_FILENO){
                    if(dup2(pfd2[0],STDIN_FILENO)==-1)
                        perror("dup2 1");
                    if(close(pfd2[0])==-1)
                        perror("close 2");
                }

                ////islemin basladgı yer
                //char buffer[15];
                //fgets(buffer, 15 , stdin);
                //printf("aldim(PROCESS3) :%s",buffer);
                myexecV(args[5],NULL);
                ////islemın bttgi yer
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
        ///////////PROCESS 3 end


        if(close(pfd1[0])== -1)
            perror("close");
        if(close(pfd1[1])== -1)
            perror("close");
        if(close(pfd2[0])== -1)
            perror("close");
        if(close(pfd2[1])== -1)
            perror("close");

        for(int i=0; i<count; i++){
            wait(NULL);
        }
    }

    return 1;
}
int exec(char **args, int count)
{
    int i;
    if (args[0] == NULL) {
        return 1;
    }

    return launch(args, count);
}
char *readLine(void)
{
    int bufsize = 1024;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        exit(EXIT_FAILURE);
    }
    while (1) {
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufsize) {
            bufsize += 1024;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
char **splitFunction(char *line, int *count)
{
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIMETERS);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += 64;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL,DELIMETERS);
    }
    *count=position;
    tokens[position] = NULL;
    return tokens;
}

void execute(void)
{
    char *line;char **args;
    int status;int count=0;
    do {
        printf("$>>");
        line = readLine();
        args = splitFunction(line, &count);
        status = exec(args, count);
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    execute();
    return EXIT_SUCCESS;
}