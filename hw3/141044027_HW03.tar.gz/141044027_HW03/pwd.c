#include <unistd.h>
#include <stdio.h>

int main(int argc,char *argv[]) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
}
