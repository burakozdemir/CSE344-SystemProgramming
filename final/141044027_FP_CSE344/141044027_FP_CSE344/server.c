#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include<arpa/inet.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>   //for threading , link with lpthread
#include <sys/time.h>

struct provider{
    int id;
    char name[20];
    int performance;
    int price;
    int duration;
    int socket;
};

struct client{
    char name[20];
    char priority;
    int degree;
    char server[200];
    int port;
};

typedef struct type{
    char name[10];
    char prio[2];
    int degree;
    int socket;
    int finish;
    struct type *p;
}node;

typedef struct queue{
    int size;
    node *head;
    int limit;
    node *tail;
} queue;

void *providerThread(void *provid);
int control(const char*priority);
void *connection_handler(void *);
int execute();
int getProvidersNumber(const char* filename);
int parser(const char *filename);
static void ctrl_c(int signum);
int createThreads();
double fact(double x);
double calculate(double val);
long timediffernce(struct timeval start, struct timeval end);
/*
QUEUE DATA STRUCTURE
*/
queue *cq(int startlimit);
void dq(queue *que);
int add(node *val,queue *que);
node *poll(queue *que);
int isempty(queue *que);
int isFull(queue *que);

//////////////////////////////////////////////////////////////////777
int socket_desc , client_sock , c ;
pthread_t sniffer_thread;

int count=0;//finsher counter for waiting clients
int STOP=0;
int numberProviders=0;
int portNumber;
FILE *fptr;

queue **queues;
pthread_cond_t *condition;
pthread_mutex_t *mutexes;
pthread_t *threads;
pthread_attr_t attr;
pthread_t finisher;

// node **pn;
node *mainNode;
int *processingNumber;
int *finishSocket;
int *threadKillControls;
struct provider *providers;

int mallocAllPointers(){
    mainNode=malloc((numberProviders+1)* sizeof(node *));
    queues=malloc((numberProviders+1)*sizeof(queue *));
    condition=malloc((numberProviders+1)*sizeof(pthread_cond_t));
    mutexes=malloc((numberProviders+1)*sizeof(pthread_mutex_t));
    threads=malloc((numberProviders+1)*sizeof(pthread_t));
    processingNumber=malloc((numberProviders+1)* sizeof(int));
    finishSocket=malloc((numberProviders+1)*3*sizeof(int));
    threadKillControls=malloc((numberProviders)* sizeof(int));
    providers=malloc((numberProviders+1)*sizeof(struct provider));
}
int freeAllPOinters(){
    free(queues);
    free(condition);
    free(mutexes);
    free(threads);
    //free(pn);

    free(threadKillControls);
    free(processingNumber);
    free(finishSocket);
    free(providers);
    free(mainNode);
}

/**
* 1. pointerlara malloc ile yer al ve  geldgnde free ettgnden emın ol
* 2. mutex queue ve condısıton varıablelerı destroy ettgnden emın ol
* 3. threadlerın duzgun oldgunden emın ol
* 4. socket ve fileların duzgun kapatldgndan emın ol
*/

int execute(){

    for (int i = 0; i < (numberProviders+1)*3; ++i) {
        finishSocket[i]=-1;
    }

    for (int j = 0; j < numberProviders; ++j) {
        processingNumber[j]=0;
    }

    struct sockaddr_in server , client;

    memset(&server,0,sizeof(struct sockaddr_in));

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }

    int cntrl=1;

    /*
    if( setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&cntrl, sizeof(cntrl)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }*/

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( portNumber );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        close(socket_desc);
        close(client_sock);
        freeAllPOinters();
        fclose(fptr);
        perror("bind failed. Error");
        return 1;
    }
    printf("%d provider threads created\n",numberProviders);
    printProvider();
    createThreads(providers);

    if (listen(socket_desc, 50) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    c = sizeof(struct sockaddr_in);
    int res;
    pthread_attr_t attr;
    res = pthread_attr_init(&attr);
    if (res != 0) {
        perror("Attribute init failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached state failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is waiting for client connections at port %d\n",portNumber);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
        char mesage[50];
        strcpy(mesage,"NO PROVIDER IS LIVE.SERVER IS SHUTTING DOWN\n");
        if(STOP==1){
            if( send(client_sock , mesage , strlen(mesage) , 0) < 0) {
                printf("send failed(ctrl_C):%d\n",client_sock);
            }
            closeALL();
            return 1;
        }

        int temp;
        temp=client_sock;
        if( pthread_create(&sniffer_thread , &attr ,  connection_handler , (void *)temp) < 0) {
            perror("could not create thread");
            return 1;
        }
        //printf("Server is waiting for client connections at port %d\n",portNumber);
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}
int closeALL(){
    close(socket_desc);
    close(client_sock);
    pthread_attr_destroy(&attr);
    for (int i = 0; i < numberProviders; ++i) {
        if(threadKillControls[i]==0){
            while(!isempty(queues[i])){
                node *temp=poll(queues[i]);
                free(temp);
            }
            dq(queues[i]);
            pthread_mutex_destroy(&mutexes[i]);
            pthread_cond_destroy(&condition[i]);
            pthread_cancel(threads[i]);
        }
    }
    pthread_cancel(finisher);
    printResult();
    freeAllPOinters();
    fclose(fptr);
}
int main(int argc , char *argv[]){
    if (argc != 4) {
        perror("Usage: $./a.out <portnumber> <providers_file> <logfile>\n");
        return -1;
    }

    srand(time(NULL));
    portNumber=atoi(argv[1]);
    char providersfile[20];
    strcpy(providersfile,argv[2]);
    char logfile[20];
    strcpy(logfile,argv[3]);

    ///MODUNA DIKKAT ETttttttttttttttttttt
    fptr = fopen(logfile, "w");

    if(fptr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act4;
    memset (&act4, '\0', sizeof(act4));
    act4.sa_handler = &ctrl_c;

    if (sigaction(SIGINT, &act4, NULL) < 0) {
        fclose(fptr);
        perror ("sigaction");
        return 1;
    }

    numberProviders=getProvidersNumber(providersfile);
    if(numberProviders<=0){
        fclose(fptr);
        perror("numbers of provider is less than 1");
        exit(EXIT_FAILURE);
    }
    mallocAllPointers();
    parser(providersfile);
    printf("Logs kept at %s\n",providersfile);
    fprintf(fptr,"Logs kept at %s\n",providersfile);
    execute(portNumber,logfile);
 }
static void ctrl_c(int signum){
    if (signum == SIGINT) {
        //free(SEQ);
        char mss[20]="";
        strcpy(mss,"SERVER SHUTDOWN\n");
        for (int j = 0; j <numberProviders*3 ; ++j) {
            //printf("%d\n",finishSocket[j]);
            if(finishSocket[j]!=-1){
                if( send(finishSocket[j] , mss , strlen(mss) , 0) < 0) {
                    printf("send failed(ctrl_C):%d\n",finishSocket[j]);
                }
            }
        }
        closeALL();
        exit(1);
    } else {
        printf("Not a SIGINT signal: %d (%s)\n", signum, strsignal(signum));
    }
}
int killThread(int id){
    //pthread_mutex_destroy(&mutexes[id]);
    //pthread_cond_destroy(&condition[id]);
    if(providers[id].socket!=-1){
        char mss[50]="";
        strcpy(mss,"Your business is stayed in half\n");
        if( send(providers[id].socket , mss , strlen(mss) , 0) < 0) {
            printf("send failed(ctrl_C):%d\n",providers[id].socket);
        }
    }

    while(!isempty(queues[id])){
        node *temp=poll(queues[id]);
        if(temp->socket!=-1){
            char mss[20]="";
            strcpy(mss,"Provider is killed by SIGNAL\n");
            if( send(temp->socket , mss , strlen(mss) , 0) < 0) {
                printf("send failed(ctrl_C):%d\n",temp->socket);
            }
        }
        free(temp);
    }
    dq(queues[id]);
    pthread_cancel(threads[id]);
    threadKillControls[id]=1;
    printf("%s is dead\n",providers[id].name);
}
void *connection_handler(void *socket_desc){
    //Get the socket descriptor
    //adasdsadsadasd
    int sock = (int)socket_desc;
    int read_size;
    char *message , client_message[2000];
    int MAXCLIENTNUMBER = numberProviders*3;

    //Receive a message from client
    if((read_size = recv(sock , client_message , 2000 , 0)) > 0){
        perror(client_message);
        char name[50]="";
        char pri[50]="";
        int degree;
        int res=-1;
        sscanf(client_message,"%s %s %d,",name,pri,&degree);
        res=control(pri);

        if(res!=-1){
            printf("Client %s (%s %d) connected, forwarded to provider %s\n",name,pri,degree,providers[res].name);
            fprintf(fptr,"Client %s (%s %d) connected, forwarded to provider %s\n",name,pri,degree,providers[res].name);

            if(count>=MAXCLIENTNUMBER)
                count=0;
            //ctrl_c durumunda socket blgisi
            finishSocket[count]=sock;


            node *n;
            pthread_mutex_lock(&mutexes[res]);
            //printf("(before add) queue(%d):%d \n",res,queues[res]->size);
            n = (node*) malloc(sizeof (node));
            strcpy(n->name,name);
            strcpy(n->prio,pri);
            n->degree=degree;
            n->socket=sock;
            n->finish=count;
            count++;
            add(n,queues[res]);

            pthread_cond_signal(&condition[res]);
            pthread_mutex_unlock(&mutexes[res]);

        }else{
            char mss[200]="";
            strcpy(mss,"NO PROVIDER IS NOT AVAILABLE\n");
            fprintf(fptr,"NO PROVIDER IS NOT AVAILABLE\n");
            if( send(sock , mss , strlen(mss) , 0) < 0)
            {
                puts("Send failed(provider)");
            }
        }
        //Send the message back to client
        //write(sock , client_message , strlen(client_message));

    }else{
        perror("wrong recv function\n");
    }

    pthread_exit(NULL);
}
void *providerThread(void *provid){
    struct provider *prov=(struct provider*)provid;
    char mss[500]="";
    struct timeval start, end0,end1;
    double t0,t1;
    providers[prov->id].socket=-1;

    while(1){
        pthread_mutex_lock(&mutexes[prov->id]);
        while(isempty(queues[prov->id])){
            printf("Provider %s waiting for tasks\n",prov->name);
            fprintf(fptr,"Provider %s waiting for tasks\n",prov->name);
            pthread_cond_wait(&condition[prov->id],&mutexes[prov->id]);
        }
        node *pn;
        pn = poll(queues[prov->id]);
        pn->degree=(pn->degree)*(pn->degree);
        int id=pn->degree;
        char name[20];strcpy(name,pn->name);
        char p[2];strcpy(p,pn->prio);
        int degree=pn->degree;
        int sendsocket=pn->socket;
        int finish=pn->finish;
        providers[prov->id].socket=sendsocket;
        free(pn);
        pthread_mutex_unlock(&mutexes[prov->id]);
        gettimeofday(&start, NULL);
        printf("Provider %s is processing task number %d: %d\n",prov->name,processingNumber[prov->id],degree);
        fprintf(fptr,"Provider %s is processing task number %d: %d\n",prov->name,processingNumber[prov->id],degree);

        int random=rand()%10 + 5;
        pthread_yield(NULL);
        sleep(random);
        double res=calculate((degree*3.1415)/180);
        gettimeofday(&end0, NULL);
        t0 = timediffernce(start,end0)/1000.0;

        printf("Provider %s completed task number %d: cos(%d)=%.3lf in %.3lf seconds.\n",prov->name,processingNumber[prov->id],degree,res,t0);
        fprintf(fptr,"Provider %s completed task number %d: cos(%d)=%.3lf in %.3lf seconds.\n",prov->name,processingNumber[prov->id],degree,res,t0);

        gettimeofday(&end1, NULL);
        t1 = timediffernce(start,end1)/1000.0;

        sprintf(mss,"%s’s task completed by %s in %d seconds, cos(%d)=%.3lf, cost is %dTL , total time spent %.3lf seconds\n", name,prov->name,random,degree,res,prov->price,t1);
        fprintf(fptr,"%s’s task completed by %s in %d seconds, cos(%d)=%.3lf, cost is %dTL , total time spent %.3lf seconds\n",name,prov->name,random,degree,res,prov->price,t1);
        if( send(sendsocket, mss , strlen(mss) , 0) < 0)
        {
            puts("Send failed(provider)");
            return (void *)1;
        }
        providers[prov->id].socket=-1;
        finishSocket[finish]=-1;
        processingNumber[prov->id]+=1;

    }
    pthread_exit(NULL);
}
int isallThreadKill(){
    int count=0;
    for (int i = 0; i < numberProviders; ++i) {
        if(threadKillControls[i]==1)
            count++;
    }
    if(count==numberProviders)return 1;
    else return 0;
}
void *finisherThread(void *param) {
    int count=0;
    while(1){
        for (int i = 0; i < numberProviders; ++i) {
            if(count>=providers[i].duration && threadKillControls[i]==0)
                killThread(i);
        }
        sleep(1);
        count++;
        if(isallThreadKill()){
            STOP=1;
            pthread_exit(NULL);
        }
    }
}
void printProvider(){
    printf("Name       Performance       Price      Duration\n");
    fprintf(fptr,"Name       Performance       Price      Duration\n");
    for (int i = 0; i < numberProviders; ++i) {
        printf("%s        %d        %d          %d\n",providers[i].name,providers[i].performance,providers[i].price,providers[i].duration);
        fprintf(fptr,"%s        %d        %d          %d\n",providers[i].name,providers[i].performance,providers[i].price,providers[i].duration);
    }
}
void printResult(){
    printf("Termination signal received\n");fprintf(fptr,"Termination signal received\n");
    printf("Terminating all cilents\n");fprintf(fptr,"Terminating all cilents\n");
    printf("Terminating all providers\n");fprintf(fptr,"Terminating all providers\n");
    printf("Statics\n");fprintf(fptr,"Statics\n");
    printf("Name        Number Served\n");fprintf(fptr,"Name        Number Served\n");
    for (int i = 0; i < numberProviders; ++i) {
        printf("%s          %d\n",providers[i].name,processingNumber[i]);
        fprintf(fptr,"%s          %d\n",providers[i].name,processingNumber[i]);
    }
    printf("Goodbye\n");
    fprintf(fptr,"Goodbye\n");
}
int createThreads(){

    for (int i = 0; i < numberProviders; ++i){
        queues[i]=cq(2);
        pthread_mutex_init(&mutexes[i],0);
        pthread_cond_init(&condition[i],0);
        threadKillControls[i]=0;
    }


    int res,err;

    res = pthread_attr_init(&attr);
    if (res != 0) {
        perror("Attribute init failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached state failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numberProviders; ++i){
        providers[i].id=i;
        pthread_create(&threads[i],&attr,providerThread,&providers[i]);
    }

    pthread_create(&finisher,&attr,finisherThread,NULL);


}
int helper(int flag,int *values,int *indexs){
    if(flag==1){
        for (int i = 0; i < numberProviders; ++i) {
            for (int j = i + 1; j < numberProviders; ++j){
                if (values[i] > values[j]) {
                    int a =  values[i];
                    values[i] = values[j];
                    values[j] = a;

                    int k=indexs[i];
                    indexs[i]=indexs[j];
                    indexs[j]=k;
                }
            }
        }
    }else{
        for (int i = 0; i < numberProviders; ++i) {
            for (int j = i + 1; j < numberProviders; ++j){
                if (values[i] < values[j]) {
                    int a =  values[i];
                    values[i] = values[j];
                    values[j] = a;

                    int k=indexs[i];
                    indexs[i]=indexs[j];
                    indexs[j]=k;
                }
            }
        }
    }
}
int sirala(int *indexes,int *values,int cmp){
    if(cmp==1){
        helper(1,values,indexes);
    }
    else if(cmp==2){
        helper(0,values,indexes);
    }
    else if(cmp==3){
        helper(1,values,indexes);
    }else{
        printf("wrong priority command:%d\n",cmp);
    }
}
int control(const char*priority){
    int cmp=0;
    //printf("%s\n",priority);
    if(strcmp(priority,"C")==0)
        cmp=1;
    if(strcmp(priority,"Q")==0)
        cmp=2;
    if(strcmp(priority,"T")==0)
        cmp=3;

    if(cmp==0)
        return -1;

    int provIndexes[7];
    int provValues[7];

    for (int i = 0; i < numberProviders; ++i){
        provIndexes[i]=i;
        if(cmp==1)
            provValues[i]=providers[i].price;
        if(cmp==2)
            provValues[i]=providers[i].performance;
        if(cmp==3)
            provValues[i]=providers[i].duration;
    }

    sirala(provIndexes,provValues,cmp);

    for (int i = 0; i < numberProviders; ++i)
    {
        //printf("prov(%d) value:%d \n",provIndexes[i],provValues[i]);
    }
    int index=-1;
    //printf("adad\n");

    for (int i = 0; i < numberProviders; ++i){
        if(threadKillControls[provIndexes[i]]==0){
            pthread_mutex_lock(&mutexes[provIndexes[i]]);
            if(!isFull(queues[provIndexes[i]])) {
                index = provIndexes[i];
                pthread_mutex_unlock(&mutexes[provIndexes[i]]);
                break;
            }
            pthread_mutex_unlock(&mutexes[provIndexes[i]]);
        }
    }

    //printf("thread(%d) yonlendirildi\n",index);
    return index;

}
double fact(double x) {
    if(x == 0)
        return 1;
    if(x == 1)
        return 1;
    return x * fact(x-1);
}
double calculate(double val){
    int k = 20;
    double result = 0.0;
    for(int i = 0 ; i<k ; i++) {
        result += pow(-1.0, i) * pow(val, 2.0 * i)/fact(2.0*i);
    }
    return result;
}
long timediffernce(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec)/* / 1000.0f*/;
}
int getProvidersNumber(const char* filename){
    FILE * fp;
    int clientsCount=0;
    struct provider p;

    fp = fopen(filename, "r");
    if (fp == NULL){
        perror("Wrong provider file");
        exit(EXIT_FAILURE);
    }

    char temp[20];
    //Ertan 4 600 120
    fscanf(fp,"%s %s %s %s",temp,temp,temp,temp);

    while(!feof(fp)){
        fscanf(fp,"%s %d %d %d",p.name,&p.performance,&p.price,&p.duration);
        //printf("%s %d %d %d\n",p.name,p.performance,p.price,p.duration);
        clientsCount++;
    }

    fclose(fp);
    return clientsCount-1;
}
int parser(const char *filename){
    FILE * fp;
    int clientsCount=0;
    struct provider p;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char temp[20];
    //Ertan 4 600 120
    fscanf(fp,"%s %s %s %s",temp,temp,temp,temp);
    int i=0;
    while(!feof(fp)){
        providers[i].id=i;
        fscanf(fp,"%s %d %d %d",providers[i].name,&(providers[i].performance),&(providers[i].price),&(providers[i].duration));
        //printf("%s %d %d %d\n",providers[i].name,providers[i].performance,providers[i].price,providers[i].duration);
        i++;
        clientsCount++;
    }

    fclose(fp);
}
queue *cq(int startlimit){
    queue *que=(queue *)malloc(sizeof(queue));
    if(que==NULL)return NULL;
    if(startlimit<=0)startlimit=1000;
    que->limit=startlimit;
    que->tail=NULL;
    que->head=NULL;
    que->size=0;
}
void dq(queue *que){
    node *nod;
    while(!isempty(que)){
        nod=poll(que);
        free(nod);
    }
    free(que);
}
int add(node *val,queue *que){
    if(que==NULL || val==NULL)return 0;
    if(que->size>=que->limit)return 0;
    val->p=NULL;
    if(que->size==0){
        que->head=val;
        que->tail=val;
    }else{
        que->tail->p=val;
        que->tail=val;
    }
    que->size++;
    return 1;
}
node *poll(queue *que){
    node *temp;
    if(isempty(que))return NULL;
    temp=que->head;
    que->head=(que->head)->p;
    que->size--;
    return temp;
}
int isempty(queue *que){
    if(que==NULL)return 0;
    if(que->size==0)return 1;
    else return 0;
}
int isFull(queue *que){
    if(que->size==2)return 1;
    else return 0;
}
