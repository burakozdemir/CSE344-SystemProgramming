//signal example
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

/////////////////////////////7
static int lineCount = 0;
int CHILD=0;
FILE *filepoint;
/////////////////////
/**
 * writeEndOfFile
 */
FILE *parentLogFile;
//////////////////////////7

/**
 * deleteToLİne
 */
FILE *childLogg;
////////////////////////
float getRandNum(){
    float x = 0 + (rand() / ( RAND_MAX / (10-0) ) ) ;
    return x;
}

static void waitChild(int signum){
    if (signum == SIGCHLD) {

    } else {
        printf("Not a SIGCHLD signal: %d (%s)\n", signum, strsignal(signum));
    }
}

static void ctrl_c(int signum){
    if (signum == SIGINT) {
        if(filepoint!=NULL)fclose(filepoint);
        if(CHILD){
            if(childLogg!=NULL)fclose(childLogg);
        }
        else{
            if(parentLogFile!=NULL)fclose(parentLogFile);
            wait(NULL);
        }
        //free(SEQ);
        exit(1);
    } else {
        printf("Not a SIGINT signal: %d (%s)\n", signum, strsignal(signum));
    }
}
//SIGUSR1 yakalar
static void parentHandler (int signum)
{
    if (signum == SIGUSR2) {
        lineCount--;
    } else {
        printf("Not a SIGUSR1 signal: %d (%s)\n", signum, strsignal(signum));
    }
}
//SIGUSR2 yakalar
static void childHandler (int signum)
{
    if (signum == SIGUSR1) {
        lineCount++;
    } else {
        printf("Not a SIGUSR2 signal: %d (%s)\n", signum, strsignal(signum));
    }
}

int myParser(char* A,int N,float *res){
    const char s[2] = " ";
    char *token;
    int i=0;
    /* get the first token */
    token = strtok(A, s);
    res[i]=(float)atof(token);
    i++;
    /* walk through other tokens */
    while( i< N && token != NULL ) {
        token = strtok(NULL, s);
        if(token!=NULL)
            res[i]=(float)atof(token);
        i++;
    }
}
int parentLog(float seq[],int N,int lineIndis){
    fprintf(parentLogFile,"Parent produce this sequence==> (%d): ",lineIndis);
    for (int i = 0; i < N; ++i) {
        fprintf(parentLogFile,"%0.2lf ",seq[i]);
    }
    fprintf(parentLogFile,"\n");
}
int childLog(char *line,int N,int lineIndis){
    fprintf(childLogg,"Child delete this sequence==> (%d): ",lineIndis);
    fprintf(childLogg,"%s",line);
    fprintf(childLogg,"\n");
}
//https://stackoverflow.com/questions/19429138/append-to-the-end-of-a-file-in-c
int writeEndOfFile(const char* filename,int N){
    sigset_t sigset={};
    sigset_t old={};
    sigaddset(&sigset,SIGINT);
    sigprocmask(SIG_BLOCK,&sigset,&old);
    //////////////////////////
    FILE *fp=fopen(filename,"a");
    float *a=malloc(sizeof(float)*N);

    for (int i = 0; i < N; ++i) {
        float temp=getRandNum();
        fprintf(stdout,"%.2lf ",temp);
        a[i]=temp;
        fprintf(fp,"%.2lf ",temp);
    }
    fprintf(fp,"\n");
    parentLog(a,N,lineCount);

    free(a);
    fclose(fp);

    ////////////////////////////
    sigprocmask(SIG_UNBLOCK,&old,NULL);
}
//https://batchloaf.wordpress.com/2013/12/07/simple-dft-in-c/
int calculateDFT(int N, float *sequence, float *reel, float *imag){
    int n, k;             // indices for time and frequency domains
    float x[N],PI2=6.2832;           // discrete-time signal, x

    // Calculate DFT of x using brute force
    for (k=0 ; k<N ; ++k)
    {
        // Real part of X[k]
        reel[k] = 0;
        for (n=0 ; n<N ; ++n) reel[k] += sequence[n] * cos(n * k * PI2 / N);

        // Imaginary part of X[k]
        imag[k] = 0;
        for (n=0 ; n<N ; ++n) imag[k] -= sequence[n] * sin(n * k * PI2 / N);

    }
}
//r+ modunda bir fp alıyor
int deleteEnd(FILE *fp,int N,int M){
    sigset_t sigset={};
    sigset_t old={};
    sigaddset(&sigset,SIGINT);
    sigprocmask(SIG_BLOCK,&sigset,&old);
    int calculate=N*4 + N + 1;
    fseek(fp,0,SEEK_END);
    int cntrl=ftell(fp);

    if(cntrl!=0){
        /*
        fseek(fp,-calculate,SEEK_END);
        char *temp= (char*) malloc(sizeof(char) * calculate);
        int i=0;
        do
        {
            temp[i]= (char)fgetc(fp);
            i++;
        } while( temp[i] != EOF);

        float *res=malloc(sizeof(float)*N);
        float *reel=malloc(sizeof(float)*N);
        float *image=malloc(sizeof(float)*N);
        myParser(temp,N,res);
        childLog(temp,N,lineCount);

        calculateDFT(N,res,reel,image);
        for (int j = 0; j < N; ++j) {
            fprintf(stdout,"%.1lf",reel[j]);
            if(image[j]>0)
                fprintf(stdout,"+");
            fprintf(stdout,"%.1lf",image[j]);
            fprintf(stdout," ");
        }
        fprintf(stdout,"\n");
        ///////////////
        free(temp);
        free(res);
        free(reel);
        free(image);
        */
        ////////////////////
        fseek(fp,-calculate,SEEK_END);
        int newEOF=(int)ftell(fp);
        ftruncate(fileno(fp),newEOF);

    }
    ////////////////////////////
    sigprocmask(SIG_UNBLOCK,&old,NULL);

}
int DFT(int N,const char *filename,int M){
    struct flock fl;
    int child;
    //fflush(0);
    switch ((child=fork())){
        case -1:
            perror("Fork Error");
            break;
            /**
             * Child Process
             */
        case 0:
            CHILD=1;
            //perror("*****************************\n");
            filepoint=fopen(filename,"a+");
            childLogg=fopen("childlog.txt","a+");
            struct sigaction act;
            memset (&act, '\0', sizeof(act));
            act.sa_handler = &childHandler;
            if (sigaction(SIGUSR1, &act, NULL) < 0) {
                perror ("sigaction");
                return 1;
            }

            struct sigaction act2;
            memset (&act2, '\0', sizeof(act2));
            act2.sa_handler = &ctrl_c;

            if (sigaction(SIGINT, &act2, NULL) < 0) {
                perror ("sigaction");
                return 1;
            }
            kill(getppid(),SIGCHLD);
            if(filepoint!=NULL){
                fl.l_type = F_WRLCK; //type is write lock
                fl.l_whence = SEEK_SET; //starting from the beginning of the file
                fl.l_start = 0; //no offset
                fl.l_len= 0; //to the end of the file. Note that 0 means to the EOF.
                fl.l_pid = getpid(); //current process id.
                while(1){
                    //sleep(2);
                    if(lineCount==0){ if (sigsuspend(&act.sa_mask) == -1) {}}

                    fl.l_type=F_WRLCK;
                    //perror("Child key istiyor");
                    if (fcntl(fileno(filepoint), F_SETLKW, &fl) == -1) {}
                    //perror("Child kilidi aldı.");
                    /////////////////////////
                    //perror("Process B: the dft of line: %d:\n");
                    fprintf(stdout,"Process B: dft is this for line %d:",lineCount+1);
                    fflush(stdout);
                    deleteEnd(filepoint,N,M);
                    fprintf(stdout,"\n");
                    fflush(stdout);
                    //////////////////////////
                    kill(getppid(),SIGUSR2);
                    lineCount--;
                    fl.l_type=F_UNLCK;
                    fcntl(fileno(filepoint),F_SETLKW,&fl);
                    //perror("Child kilidi bıraktı.");
                }
            }
            break;
            /**
             * Parent Process
             */
        default:
            parentLogFile=fopen("parentlog.txt","a+");
            filepoint=fopen(filename,"a+");
            struct sigaction act3;
            memset (&act3, '\0', sizeof(act3));
            act3.sa_handler = &parentHandler;

            if (sigaction(SIGUSR2, &act3, NULL) < 0) {
                perror ("sigaction");
                return 1;
            }

            struct sigaction act4;
            memset (&act4, '\0', sizeof(act4));
            act4.sa_handler = &ctrl_c;

            if (sigaction(SIGINT, &act4, NULL) < 0) {
                perror ("sigaction");
                return 1;
            }

            struct sigaction act5;
            memset (&act5, '\0', sizeof(act5));
            act5.sa_handler = &waitChild;

            if (sigaction(SIGCHLD, &act5, NULL) < 0) {
                perror ("sigaction");
                return 1;
            }

            if (sigsuspend(&act5.sa_mask) == -1){}

            if(filepoint!=NULL){
                fl.l_type = F_WRLCK; //type is write lock
                fl.l_whence = SEEK_SET; //starting from the beginning of the file
                fl.l_start = 0; //no offset
                fl.l_len= 0; //to the end of the file. Note that 0 means to the EOF.
                fl.l_pid = getpid(); //current process id.

                while (1){
                    //sleep(2);
                    if(lineCount==M){ if (sigsuspend(&act3.sa_mask) == -1){}}

                    fl.l_type=F_WRLCK;
                    //perror("Parent key istiyor");
                    if (fcntl(fileno(filepoint), F_SETLKW, &fl) == -1) {}

                    ////////////////////////
                    fprintf(stdout,"Process A: i’m producing a random sequence for line %d:",lineCount+1);
                    fflush(stdout);
                    //perror("Process A ");
                    writeEndOfFile(filename,N);
                    fprintf(stdout,"\n");
                    fflush(stdout);
                    kill(child,SIGUSR1);
                    lineCount++;
                    //////////////////////////


                    fl.l_type=F_UNLCK;
                    fcntl(fileno(filepoint),F_SETLKW,&fl);
                    //perror("Parent kilidi bıraktı.");
                }
            }
            break;
    }
    return 1;
}

/**
 * Program result.txt dosyasında calısıyor process A Yazıyor process B siliyor.
 * Girilen M degeri dogrultusunda sınırı asmiyor.
 * Ama process B sequenci isleme aldgnda DFT hesaplarken oluyor ve process A sonsuza gdiyor
 * @param argc
 * @param argv
 * @return
 */
int main(int argc,  char * argv[]){

    if (argc != 7) {
        perror("Usage: $./multiprocess_DFT -N 5 -X <filename> -M 100 \n");
        return -1;
    }
    int N=atoi(argv[2]);
    int M=atoi(argv[6]);
    const char* filename=argv[4];

    if(N<1 || N>20){
        perror("Width is not true range");
    }
    if(M<1 || M>1000){
        perror("Height is not true range");
    }
    srand(time(NULL));
    /*
    FILE *a=fopen(filename,"a+");
    writeEndOfFile(filename,N);
    writeEndOfFile(filename,N);
    writeEndOfFile(filename,N);
    deleteEnd(a,N,M);
    fclose(a);*/
    DFT(N,filename,M);
    //printf("%d\n",control(filename));
    //delete(filename,N);
    //printf("%0.1f \n",getRandNum());
    exit(EXIT_SUCCESS);
}
