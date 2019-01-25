#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define EGGS 1
#define FLOUR 2
#define BUTTER 3
#define SUGAR 4
#define NUMBER 6
#define SIZE 1024

struct chef{
   int id;
   int  processID;
   int  eksik1;
   int 	eksik2;
};
const char *name = "sharedMemName";
sem_t *eggs=NULL;
sem_t *flour=NULL;
sem_t *butter=NULL;
sem_t *sugar=NULL;
sem_t *sem=NULL;
struct chef chefs[NUMBER];

int shm_fd;
void *ptr;

static void ctrl_c(int signum){
    if (signum == SIGINT) {
        printf("process get SIGINT\n");
        close(shm_fd);
        sem_close(sem);
        sem_close(eggs);
        sem_close(flour);
        sem_close(butter);
        sem_close(sugar);
        exit(1);
    } else {
        printf("Not a SIGINT signal: %d (%s)\n", signum, strsignal(signum));
    }
}

int execute();
int getID(int pid){
	for(int i=0;i<NUMBER;i++){
		if(chefs[i].processID==pid)
			return i;
	}
	return -1;
}

int main(int argc,char *argv[]){
	srand(time(NULL));
	execute(NUMBER);
	return 0;
}

char *getItem(int i){
	char *res;
	switch(i){
	case 1:res="eggs";break;
	case 2:res="flour";break;
	case 3:res="butter";break;
	case 4:res="sugar";break;
	default:res="non";break;
	}
	return res;
}

void *proc_create(int *parent) { 
    *parent=getpid();
    for (int i = 0; i < NUMBER; i++) {
        int pid = fork();
        if (pid < 0) {
            perror("Fork Failed!");
            exit(1);
        } else if (pid == 0) {
        	chefs[i].processID = getpid();
			chefs[i].id=i;
            return NULL;
        } else {            
        }
    }
}

void duzenle(){
		chefs[0].eksik1=1;chefs[0].eksik2=2;
		chefs[1].eksik1=1;chefs[1].eksik2=3;
		chefs[2].eksik1=1;chefs[2].eksik2=4;
		chefs[3].eksik1=2;chefs[3].eksik2=3;
		chefs[4].eksik1=2;chefs[4].eksik2=4;
		chefs[5].eksik1=3;chefs[5].eksik2=4;

}
int isEmptySharedMem(){
	sem_wait(sem);
	char *res;
	/*
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed(isEmpty)\n");
		return -1;
	}*/
	res=ptr;
	char x;
	x=*res;
	sem_post(sem);
	//printf("okunandeger(isEmpty):%d\n",(int)x-48);
	if(((int)x-48)==0){
		return 1;
	}else
		return 0;	
}

int returnValueOfSharedMem(int *eksik1,int *eksik2){
	sem_wait(sem);
	char *res;
	/*
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed(returnvalueofsharedmem)\n");
		return -1;
	}*/
	res=ptr;
	char x;
	x=*res;
	//printf("%d\n",(int)x-48);
	if(((int)x-48)!=0){
		//printf("asdad\n");
		char a,b;
		a=*res;res++;
		b=*res;

		*eksik1=((int)a-48);
		*eksik2=((int)b-48);
	}
	sem_post(sem);
	return 1;
}	

int isMatched(int pid,int id,int eksik1,int eksik2,int *returnid){
	//printf("pid:%d,id:%d,eks1:%d,eks2:%d\n",pid,id,eksik1,eksik2);
	if((eksik1==chefs[getID(getpid())].eksik1 && eksik2==chefs[getID(getpid())].eksik2) || 
			(eksik2==chefs[getID(getpid())].eksik1 && eksik1==chefs[getID(getpid())].eksik2)){
			//printf("pid:%d,id:%d,eks1:%d,eks2:%d\n",pid,id,eksik1,eksik2);
			{
				*returnid=getID(getpid());
				return 1;		
			}
	}
	else 
		return 0;
}

void clearMem(){
	sem_wait(sem);
	void *temp=ptr;
	/*
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed(clearMem)\n");
		return ;
	}*/
	sprintf(temp,"%d",0);
	sem_post(sem);	
}

int postSem(int val){
	int temp;
	if(val==1){
		sem_getvalue(eggs,&temp);
		//printf("%d de postBefore.degeri:%d\n",val,temp);
		sem_post(eggs);
		sem_getvalue(eggs,&temp);
		//printf("%d den postAfter.degeri:%d\n",val,temp);
	}
	if(val==2){
		sem_getvalue(flour,&temp);
		//printf("%d de postBefore.degeri:%d\n",val,temp);
		sem_post(flour);
		sem_getvalue(flour,&temp);
		//printf("%d den postAfter.degeri:%d\n",val,temp);
	}
	if(val==3){
		sem_getvalue(butter,&temp);
		//printf("%d de postBefore.degeri:%d\n",val,temp);
		sem_post(butter);
		sem_getvalue(butter,&temp);
		//printf("%d den postAfter.degeri:%d\n",val,temp);
	}
	if(val==4){
		sem_getvalue(sugar,&temp);
		//printf("%d de postBefore.degeri:%d\n",val,temp);
		sem_post(sugar);
		sem_getvalue(sugar,&temp);
		//printf("%d den postAfter.degeri:%d\n",val,temp);
	}
}

int waitSem(int val){
	int temp;
	if(val==1){
		sem_getvalue(eggs,&temp);
		//printf("%d de tikandi.degeri:%d\n",val,temp);
		sem_wait(eggs);
		sem_getvalue(eggs,&temp);
		//printf("%d den gecti.degeri:%d\n",val,temp);
	}
	if(val==2){
		sem_getvalue(flour,&temp);
		//printf("%d de tikandi.degeri:%d\n",val,temp);
		sem_wait(flour);
		sem_getvalue(flour,&temp);
		//printf("%d den gecti.degeri:%d\n",val,temp);
	}
	if(val==3){
		sem_getvalue(butter,&temp);
		//printf("%d de tikandi.degeri:%d\n",val,temp);
		sem_wait(butter);
		sem_getvalue(butter,&temp);
		//printf("%d den gecti.degeri:%d\n",val,temp);
	}
	if(val==4){
		sem_getvalue(sugar,&temp);
		//printf("%d de tikandi.degeri:%d\n",val,temp);
		sem_wait(sugar);
		sem_getvalue(sugar,&temp);
		//printf("%d den gecti.degeri:%d\n",val,temp);
	}
}

void whosalerPut(){
	sem_wait(sem);
	void *tempPtr=ptr;
  	int r=rand()%5;		
	int flag=1;
	while(flag){
		if(r!=0)
			flag=0;
		else{
			r=rand()%5;
		}
	}
	sprintf(tempPtr,"%d",r);
	waitSem(r);///karısık yazma duclearrumu bunlardan cıkıyor olabılır
	tempPtr++;

	int r2=rand()%5;
	int flag2=1;
	while(flag2){
		if(r2!=r && r2!=0){
			flag2=0;
		}
		else{
			r2=rand()%5;
		}
	}
	sprintf(tempPtr,"%d",r2);
	waitSem(r2);
	printf("whosaler delivers %s and %s\n",getItem(r),getItem(r2));
	sem_post(sem);
}

int semcontrol(){
	int sem1,sem2,sem3,sem4;
	sem_getvalue(eggs,&sem1);
	sem_getvalue(flour,&sem2);
	sem_getvalue(butter,&sem3);
	sem_getvalue(sugar,&sem4);
	int res=0;
	int eksik1=chefs[getID(getpid())].eksik1;
	int eksik2=chefs[getID(getpid())].eksik2;
	if(sem1==0 && (eksik1==1 || eksik2==1))
		res++;
	if(sem2==0 && (eksik1==2 || eksik2==2))
		res++;
	if(sem3==0 && (eksik1==3 || eksik2==3))
		res++;
	if(sem4==0 && (eksik1==4 || eksik2==4))
		res++;
	if(res==2)
		return 1;//bekle
	else
		return 0;//silmeden devam
}

bool isAllSemCLear(){
	int sem1,sem2,sem3,sem4;
	sem_getvalue(eggs,&sem1);
	sem_getvalue(flour,&sem2);
	sem_getvalue(butter,&sem3);
	sem_getvalue(sugar,&sem4);
	if(sem1==1 && sem2==1 && sem3==1 && sem4==1)
		return true;
	else
		return false;
}	

int execute(int N){
	struct sigaction act2;
    memset (&act2, '\0', sizeof(act2));
    act2.sa_handler = &ctrl_c;

    if (sigaction(SIGINT, &act2, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    int parentID;//wholesaler id
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd,8);
	ptr=mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	sprintf(ptr,"%d",0);

	sem = sem_open("sem", O_CREAT|O_EXCL, 1, 1);
	eggs = sem_open("eggs", O_CREAT|O_EXCL, 1, 1);
	flour = sem_open("flour", O_CREAT|O_EXCL, 1, 1);
	butter = sem_open("butter", O_CREAT|O_EXCL, 1, 1);
	sugar = sem_open("sugar", O_CREAT|O_EXCL, 1, 1);

	sem_unlink("sem");
	sem_unlink("eggs");
	sem_unlink("flour");
	sem_unlink("butter");
	sem_unlink("sugar");

	proc_create(&parentID);

	duzenle();

	int count=0;
  	int eksik1;
  	int eksik2;
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
		if (ptr == MAP_FAILED) {
			printf("Map failed\n");
			return -1;
		}
  	while(1){
  		if(isEmptySharedMem()){
  			if(getpid()!=parentID){	
  				if(isAllSemCLear()){
	  				printf("chef%d is waiting for %s and %s \n",chefs[getID(getpid())].id,
	  				getItem(chefs[getID(getpid())].eksik1),
	  				getItem(chefs[getID(getpid())].eksik2));
	  				fflush(stdout);	
  				}else{
  				}
  			}else{
  				whosalerPut();
  			}
  		}else{
  			if(getpid()!=parentID){
  				returnValueOfSharedMem(&eksik1,&eksik2);
  				int returnID;
					if(semcontrol()){
						int returnID;
						if(isMatched(getpid(),getID(getpid()),eksik1,eksik2,&returnID)){
						sem_wait(sem);
						void *temp=ptr;
						
						sprintf(temp,"%d",0);
						
						printf("chef%d is preparing the dessert\n",chefs[getID(getpid())].id);
						printf("chef%d has delivered the dessert to the wholesaler\n",getID(getpid()));
						printf("wholesaler has obtained the dessert and left to sell it\n");
						//printf("postedItem:%s,%s\n",getItem(chefs[getID(getpid())].eksik1),getItem(chefs[getID(getpid())].eksik2));
						postSem(chefs[getID(getpid())].eksik1);
						postSem(chefs[getID(getpid())].eksik2);
						sem_post(sem);
								
						}else{

						}
					}
  			}else{
  				if(isAllSemCLear()){
  					printf("wholesaler is waiting for the dessert\n");
  				}else{

  				}
  			}
  		}
  		count++;
  	}
	if(getpid()!=parentID){
		sleep(1);
  		printf("pid %d process death \n", getpid());
  	}
  	
  	if(getpid()==parentID){
  			  if (shm_unlink(name) == -1) {
			printf("Error removing %s\n",name);
			exit(-1);
		  }
  		for(int i=0;i<N;i++){
  			wait(NULL);
  		}
  		printf("pid %d process death \n", getpid());
  	}
}


