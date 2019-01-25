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
#include <math.h>

struct florist{
    int id;
    char name[20];
    char flowers[3][20];
    int x,y;
    double time;
};


struct client{
    char name[10];
    int x,y;
    char flower[15];
};

typedef struct type{
    int x,y;
    char data[15];
    char name[10];
    struct type *p;
}node;

typedef struct queue{
    int size;
    node *head;
    int limit;
    node *tail;
} queue;


int STOP=0;
int DEAD1=0;
int DEAD2=0;
int DEAD3=0;

queue *queue1;
queue *queue2;
queue *queue3;

int res1=0,res2=0,res3=0;
int clientNum1=0,clientNum2=0,clientNum3=0;

pthread_cond_t condc1;
pthread_cond_t condc2;
pthread_cond_t condc3;

pthread_mutex_t thread1_Mutex;
pthread_mutex_t thread2_Mutex;
pthread_mutex_t thread3_Mutex;

int getClientNum(const char *filename);
int execute(struct florist *florists,struct client *clients,int clientNumber);
void *workerThread(void *arg);
void clean(char *val);
void parser(const char *filename,struct florist *florists,struct client *clients);
double euclidean(double x1, double y1, double x2, double y2);
int isHasFlower(char *flower,struct florist cicekci);
int yonlendirme(struct florist *florists,struct client musteri);
void swap(double *xp, double *yp,int *ids1,int *ids2);
void Sort(double arr[], int n,int ids[]);
int calculateTime(struct florist f,int x,int y);
queue *cq(int startlimit);
void dq(queue *que);
int add(node *val,queue *que);
node *poll(queue *que);
int isempty(queue *que);

/**
DOsya varmı yokmu formatını kontrol et
*/
int main(int argc,char *argv[]) {
    if (argc != 2) {
        perror("Usage: $./a.out <inputfile.dat>\n");
        return -1;
    }
    srand(time(NULL));
    //printf("%s\n",argv[1]);
    int clientNumber=getClientNum(argv[1]);
    //printf("%d\n",clientNumber);
    struct florist florists[3];
    struct client clients[clientNumber];
    parser(argv[1],florists,clients);
    execute(florists,clients,clientNumber);
}

int getClientNum(const char *filename){
    FILE * fp;
    int clientsCount=0;
    struct florist florists;
    struct client clients;
    if( access( filename, F_OK ) == -1 ) {
        perror("File is not exist");
        return -1;
    } 

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char temp[5];

    for (int i = 0; i < 3; ++i)
        fscanf(fp,"%s (%d,%d; %lf) %s %s %s %s",florists.name,&florists.x,&florists.y,&florists.time,temp, florists.flowers[0],florists.flowers[1],florists.flowers[2]);
    fseek(fp,1,SEEK_CUR);
    int i=0;
    while(!feof(fp)){
        fscanf(fp,"%s (%d,%d): %s",clients.name,&clients.x,&clients.y,clients.flower);
        i++;
        clientsCount++;
    }

    fclose(fp);
    return clientsCount-1;
}
int execute(struct florist *florists,struct client *clients,int clientNumber){

    queue1= cq(0);
    queue2= cq(0);
    queue3= cq(0);

    pthread_t threads[3];
    pthread_mutex_init(&thread1_Mutex,0);
    pthread_mutex_init(&thread2_Mutex,0);
    pthread_mutex_init(&thread3_Mutex,0);

    pthread_cond_init(&condc1,0);
    pthread_cond_init(&condc2,0);
    pthread_cond_init(&condc3,0);

    for(int i=0;i<3;i++){
        florists[i].id=i;
        pthread_create(&threads[i],NULL,workerThread,&florists[i]);
    }
    int count=0;
    while(count<clientNumber){
        node *n;
        switch(yonlendirme(florists,clients[count])){
            case 1:
                pthread_mutex_lock(&thread1_Mutex);
                n = (node*) malloc(sizeof (node));
                strcpy(n->data,clients[count].flower);
                strcpy(n->name,clients[count].name);
                n->x=clients[count].x;
                n->y=clients[count].y;
                add(n,queue1);
                clientNum1++;
                pthread_cond_signal(&condc1);
                pthread_mutex_unlock(&thread1_Mutex);
                break;
            case 2:
                pthread_mutex_lock(&thread2_Mutex);
                n = (node*) malloc(sizeof (node));
                strcpy(n->data,clients[count].flower);
                strcpy(n->name,clients[count].name);
                n->x=clients[count].x;
                n->y=clients[count].y;
                add(n,queue2);
                clientNum2++;
                pthread_cond_signal(&condc2);
                pthread_mutex_unlock(&thread2_Mutex);
                break;
            case 3:
                pthread_mutex_lock(&thread3_Mutex);
                n = (node*) malloc(sizeof (node));
                strcpy(n->data,clients[count].flower);
                strcpy(n->name,clients[count].name);
                n->x=clients[count].x;
                n->y=clients[count].y;
                add(n,queue3);
                clientNum3++;
                pthread_cond_signal(&condc3);
                pthread_mutex_unlock(&thread3_Mutex);
                break;
            default:
                printf("Yanlis yonlendirme\n");
                break;
        }
        count ++;
    }

    for (int i = 0; i < 3; ++i)
    {
        node *son;
        if(i==0){
            pthread_mutex_lock(&thread1_Mutex);
            son = (node*) malloc(sizeof (node));
            strcpy(son->data,"N");
            add(son,queue1);
            pthread_cond_signal(&condc1);
            pthread_mutex_unlock(&thread1_Mutex);}
        if(i==1){
            pthread_mutex_lock(&thread2_Mutex);
            son = (node*) malloc(sizeof (node));
            strcpy(son->data,"N");
            add(son,queue2);
            pthread_cond_signal(&condc2);
            pthread_mutex_unlock(&thread2_Mutex);}
        if(i==2){
            pthread_mutex_lock(&thread3_Mutex);
            son = (node*) malloc(sizeof (node));
            strcpy(son->data,"N");
            add(son,queue3);
            pthread_cond_signal(&condc3);
            pthread_mutex_unlock(&thread3_Mutex);
        }

    }

    STOP=1;

    for (int i = 0; i < 3; ++i){
        pthread_join(threads[i],NULL);

    }

    printf("----------------------------------------------\n");
    printf("Florist           #of sales          Totaltime\n");
    printf("----------------------------------------------\n");
    printf("%s                %d                 %d\n",florists[0].name,clientNum1,res1);
    printf("%s                %d                 %d\n",florists[1].name,clientNum2,res2);
    printf("%s                %d                 %d\n",florists[2].name,clientNum3,res3);
    printf("----------------------------------------------\n");

    pthread_mutex_destroy(&thread1_Mutex);
    pthread_mutex_destroy(&thread2_Mutex);
    pthread_mutex_destroy(&thread3_Mutex);

    pthread_cond_destroy(&condc1);
    pthread_cond_destroy(&condc2);
    pthread_cond_destroy(&condc3);

    dq(queue1);
    dq(queue2);
    dq(queue3);

}
void *workerThread(void *arg){
    struct florist *f=(struct florist*)arg;

    if(f->id==0){
        node *pN1;
        int val1=0;
        int count1=0;
        while(1){
            pthread_mutex_lock(&thread1_Mutex);
            while(isempty(queue1)){
                if(isempty(queue1) && STOP==1){
                    DEAD1=1;
                    pthread_exit(NULL);
                }else{
                    pthread_cond_wait(&condc1,&thread1_Mutex);
                }
            }
            pN1 = poll(queue1);
            if(strcmp(pN1->data,"N")==0){
                DEAD1=1;
                pthread_mutex_unlock(&thread1_Mutex);
                free(pN1);
                pthread_exit(NULL);
            }
            val1=calculateTime(*f,pN1->x,pN1->y);
            usleep( 100*(int)val1 );
            printf("Florist %s has delivered a %s to %s in %dms\n",f->name,pN1->data,pN1->name,val1);
            
            res1+=val1;
            free(pN1);
            pthread_mutex_unlock(&thread1_Mutex);
            count1++;
        }
    }if(f->id==1){
        node *pN2;
        int val2=0;
        int count2=0;
        while(1){
            pthread_mutex_lock(&thread2_Mutex);
            while(isempty(queue2)){
                if(isempty(queue2) && STOP==1){
                    DEAD2=1;
                    pthread_exit(NULL);
                }else{
                    pthread_cond_wait(&condc2,&thread2_Mutex);
                }
            }
            pN2 = poll(queue2);
            if(strcmp(pN2->data,"N")==0){
                DEAD2=1;
                pthread_mutex_unlock(&thread2_Mutex);
                free(pN2);
                pthread_exit(NULL);
            }
            val2=calculateTime(*f,pN2->x,pN2->y);
            usleep( 100*(int)val2 );
            printf("Florist %s has delivered a %s to %s in %dms\n",f->name,pN2->data,pN2->name,val2);
            
            res2+=val2;
            free(pN2);
            pthread_mutex_unlock(&thread2_Mutex);
            count2++;
        }
    }if(f->id==2){
        node *pN3;
        int val3=0;
        int count3=0;
        while(1){
            pthread_mutex_lock(&thread3_Mutex);
            while(isempty(queue3)){
                if(isempty(queue3) && STOP==1){
                    DEAD3=1;
                    pthread_exit(NULL);
                }else{
                    pthread_cond_wait(&condc3,&thread3_Mutex);
                }
            }
            pN3 = poll(queue3);
            if(strcmp(pN3->data,"N")==0){
                DEAD3=1;
                pthread_mutex_unlock(&thread3_Mutex);
                free(pN3);
                pthread_exit(NULL);
            }
            val3=calculateTime(*f,pN3->x,pN3->y);
            usleep( 100*(int)val3 );
            printf("Florist %s has delivered a %s to %s in %dms\n",f->name,pN3->data,pN3->name,val3);
            
            res3+=val3;
            free(pN3);
            pthread_mutex_unlock(&thread3_Mutex);
            count3++;
        }
    }
    pthread_exit(NULL);

}
void clean(char *val){
    char ch='a';
    int i=0;
    while(ch!='\0'){
        ch=val[i];
        if(ch==',')
            val[i]='\0';
        i++;
    }
}
void parser(const char *filename,struct florist *florists,struct client *clients){
    FILE * fp;
    int floristsCount=0,clientsCount=0;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char temp[5];

    for (int i = 0; i < 3; ++i) {
        fscanf(fp,"%s (%d,%d; %lf) %s %s %s %s",florists[i].name,&florists[i].x,&florists[i].y,&florists[i].time,temp, florists[i].flowers[0],florists[i].flowers[1],florists[i].flowers[2]);
        clean(florists[i].flowers[0]);
        clean(florists[i].flowers[1]);
    }

    fseek(fp,1,SEEK_CUR);

    int i=0;
    while(!feof(fp)){
        fscanf(fp,"%s (%d,%d): %s",clients[i].name,&clients[i].x,&clients[i].y,clients[i].flower);
        i++;
    }

    fclose(fp);

}

double euclidean(double x1, double y1, double x2, double y2){
    double a = x1 - x2; //calculating number to square in next step
    double b = y1 - y2;
    double distance;

    distance = pow(a, 2) + pow(b, 2);
    distance = sqrt(distance);

    return distance;
}

int isHasFlower(char *flower,struct florist cicekci){
    for (int i = 0; i < 3; ++i)
    {
        if(strcmp(cicekci.flowers[i],flower)==0)
            return 1;
    }
    return 0;
}

int yonlendirme(struct florist *florists,struct client musteri){
    double dist[3]={0,0,0};
    int floristIDs[3]={1,2,3};
    for (int i = 0; i < 3; ++i)
        dist[i]=euclidean(florists[i].x,florists[i].y,musteri.x,musteri.y);
    Sort(dist,3,floristIDs);
    for (int i = 0; i < 3; ++i){
        if(isHasFlower(musteri.flower,florists[floristIDs[i]-1]))
            return floristIDs[i];
    }
    return -1;
}

void swap(double *xp, double *yp,int *ids1,int *ids2)
{
    double temp = *xp;
    *xp = *yp;
    *yp = temp;

    int temp2=*ids1;
    *ids1 = *ids2;
    *ids2 = temp2;
}

void Sort(double arr[], int n,int ids[])
{
    int i, j, min_idx;

    // One by one move boundary of unsorted subarray
    for (int i = 0; i < 3; ++i)
    {
        for (int k = 0; k < 2; k++)
        {
            if(arr[k]>arr[k+1])
                swap(&arr[k],&arr[k+1],&ids[k],&ids[k+1]);
        }
    }
}
int calculateTime(struct florist f,int x,int y){
    double uzaklik=euclidean(f.x,f.y,x,y);
    int time=0;
    time =(rand()%50)+(10) +(int)(uzaklik/f.time);
    return time;
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
