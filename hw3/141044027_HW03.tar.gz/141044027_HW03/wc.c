#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count=0;

    fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("%s", line);
        count++;
    }

    printf("Number of line as a parametre file:%d\n",count);

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}