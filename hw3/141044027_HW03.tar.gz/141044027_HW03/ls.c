#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char ** argv) {
    struct stat sb;
    char buff[512];
    struct dirent *de;

    DIR *dr = opendir(".");

    if (dr == NULL)
    {
        printf("Could not open current directory" );
        return 0;
    }
    while ((de = readdir(dr)) != NULL){
        //int fd=fopen(de->d_name,"r");
        /*
                if (fstat(fd, &sb) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }*/
        printf("%s: ", de->d_name);
        //printf("File type: %s,",sb.st_mode);
        //printf("UID=%ld  GID=%ld\n",(long) sb.st_uid, (long) sb.st_gid);
        //printf("File size: %lld bytes\n", (long long) sb.st_size);
    }


    closedir(dr);

    exit(EXIT_SUCCESS);
}