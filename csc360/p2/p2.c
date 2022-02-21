#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

struct customer_info{
	int user_id;
	int class_type;
	int service_Time;
	int arrive_Time;
	
};
// struct customer emptyCus ={0,0,0,0};


// set up queue function
struct Queue{
	int front, rear,size,status;
	unsigned capacity;
	struct customer_info* data;
};
struct Queue* createQueue(unsigned capacity)
{
	struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->status = 0;
	queue->rear = capacity-1;
	queue->data = (struct customer_info*)malloc(sizeof(struct customer_info)*capacity);
	return queue;
};
int isFull(struct Queue* queue)
{
	return (queue->size == queue->capacity);
}
int isEmpty(struct Queue* queue)
{
	return(queue->size==0);
}
void enqueue(struct Queue* queue, struct customer_info c)
{
	if(isFull(queue))
	{
		return;
	}
	queue->rear = (queue->rear+1)%queue->capacity;
	queue->data[queue->rear] = c;
	queue->size = queue->size+1;
}
struct customer_info dequeue(struct Queue* queue)
{
	struct customer_info c = queue->data[queue->front];
	queue->front = (queue->front+1)%queue->capacity;
	queue->size = queue->size-1;
	return c;
}
struct customer_info front(struct Queue* queue)
{
	return queue->data[queue->front];
}
struct customer_info rear(struct Queue* queue)
{
	return queue->data[queue->rear];
}
// queue function end;
static struct timeval start_time;
double startUpTime;
void* customer_entry(void* cus_info);
void* clerk_entry(void* clerkNum);
pthread_mutex_t start_time_mutex = PTHREAD_MUTEX_INITIALIZER;

long convertMicroToTenth(long micro)
{
	return(micro/1e+6)*10;
}
double convertMicroToSecond(double mirco)
{
	return mirco/1e+6;
}
long convertTenthToMicro(long tenth)
{
	return(tenth/10)*1e+6;
}
double getCurrentSimulationTimeMicro()
{
	struct timeval cur_time;
	double cur_mirco_secs, init_mirco_secs;
	pthread_mutex_lock(&start_time_mutex);
	init_mirco_secs = start_time.tv_sec*1000000+(double)start_time.tv_usec;
	pthread_mutex_unlock(&start_time_mutex);
	gettimeofday(&cur_time,NULL);
	cur_mirco_secs = cur_time.tv_sec*1000000+(double)cur_time.tv_usec;
	return cur_mirco_secs-init_mirco_secs;

}
double getTotalWT(int class);

//clerk 1 mutex
pthread_mutex_t clerk1_mutex = PTHREAD_MUTEX_INITIALIZER;
//clerk 2 mutex
pthread_mutex_t clerk2_mutex = PTHREAD_MUTEX_INITIALIZER;
//clerk 3 mutex
pthread_mutex_t clerk3_mutex = PTHREAD_MUTEX_INITIALIZER;
//clerk 4 mutex
pthread_mutex_t clerk4_mutex = PTHREAD_MUTEX_INITIALIZER;

//clerk 1 condition
pthread_cond_t clerk1_cond = PTHREAD_COND_INITIALIZER;
//clerk 2 condition
pthread_cond_t clerk2_cond = PTHREAD_COND_INITIALIZER;
//clerk 3 condition
pthread_cond_t clerk3_cond = PTHREAD_COND_INITIALIZER;
//clerk 4 condition
pthread_cond_t clerk4_cond = PTHREAD_COND_INITIALIZER;

//customer mutex
pthread_mutex_t customer_b_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t customer_e_mutex = PTHREAD_MUTEX_INITIALIZER;

//customer cond
pthread_cond_t customer_b_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t customer_e_cond = PTHREAD_COND_INITIALIZER;

//clerk refact to customer
pthread_mutex_t clerk_b_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk_e_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t clerk_b_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk_e_cond = PTHREAD_COND_INITIALIZER;



int finish = 0;
int B_customer=0;
int E_customer=0;
int B_queue_status=-1;
int E_queue_status=-1;
struct Queue* B_queue,*E_queue;
double *WTimeB=NULL,*WTimeE = NULL;
int WTimeBind=0, WTimeEind = 0;

static struct timeval start_time;
double overall_waiting_time;

int main(int argc, char * argv[])
{
	int customerCount = 0;
	char* fileName = NULL;
	pthread_t* tids;
	int i;
	
	if(argc ==2)
	{
		finish = 0;
		FILE * fp = fopen(argv[1],"r");
		char lines[64];
		if(fp ==NULL)
		{
			printf("Error in file reading %s\n", argv[1]);
			return 0;
		}
		fgets(lines,sizeof(lines),fp);
		customerCount = atoi(lines);
		if(customerCount ==0)
		{
			fprintf(stderr,"the input of customer number is vaild.\n");
		}
		printf("The number of customer is: %d\n", customerCount);
		
		int pos = 0;
		pthread_t threads[customerCount];
		struct customer_info customers[customerCount];
		while(fgets(lines,sizeof(lines),fp))
		{
			
			char* token = strtok(lines,":");
			int id = atoi(token);
			if(id ==0)
			{
				printf("vaild input in id");
				pos++;
				continue;
			}
			customers[pos].user_id= id; 
			token = strtok(0,",");
			int class = atoi(token);
			if(class ==0)
			{
				B_customer++;
			}
			else if(class ==1)
			{
				E_customer++;
			}
			else
			{
				printf("vaild input in class");
				pos++;
				continue;
			}
			customers[pos].class_type =class;
			token = strtok(0,",");
			int arrive_Time = atoi(token);
			customers[pos].arrive_Time = arrive_Time;
			token = strtok(0,",");
			int service_Time = atoi(token);
			if(service_Time ==0)
			{
				printf("vaild input in service time");
				pos++;
				continue;
			}
			customers[pos].service_Time =service_Time;
			pos++;
		}
		fclose(fp);
		B_queue = createQueue(B_customer);
		E_queue = createQueue(E_customer);
		WTimeB = (double*)malloc(sizeof(double)*B_customer);
		WTimeE = (double*)malloc(sizeof(double)*E_customer);
		pthread_t clerk1_thread;
		pthread_t clerk2_thread;
		pthread_t clerk3_thread;
		pthread_t clerk4_thread;
		int clerk1 =1,clerk2 =2,clerk3 =3,clerk4 =4;
		if(pthread_create(&clerk1_thread,NULL,clerk_entry,&clerk1)!=0)
		{
			fprintf(stderr,"error: Cannot create clerk thread 1\n");
		}
		if(pthread_create(&clerk2_thread,NULL,clerk_entry,&clerk2)!=0)
		{
			fprintf(stderr,"error: Cannot create clerk thread 2\n");
		}
		if(pthread_create(&clerk3_thread,NULL,clerk_entry,&clerk3)!=0)
		{
			fprintf(stderr,"error: Cannot create clerk thread 3\n");
		}
		if(pthread_create(&clerk4_thread,NULL,clerk_entry,&clerk4)!=0)
		{
			fprintf(stderr,"error: Cannot create clerk thread 4\n");
		}
		
		for(int j =0; j< customerCount;j++)
		{
			struct customer_info *c = &customers[j];
			
			if(pthread_create(&threads[j],NULL,customer_entry,c)!=0)
			{
				fprintf(stderr,"error: Cannot create thread %d\n",j);
				return 0;
			}
		}
		startUpTime = getCurrentSimulationTimeMicro();
		if(threads)
		{
			int b;
			for(b=0;b<customerCount; b++)
			{
				pthread_join(threads[b],NULL);
			}
			finish =1;
			pthread_mutex_destroy(&customer_e_mutex);
			pthread_mutex_destroy(&customer_b_mutex);
			pthread_mutex_destroy(&clerk_b_mutex);
			pthread_mutex_destroy(&clerk_e_mutex);
			pthread_cond_destroy(&customer_b_cond);
			pthread_cond_destroy(&customer_e_cond);
			pthread_cond_destroy(&clerk_b_cond);
			pthread_cond_destroy(&clerk_e_cond);
			pthread_mutex_destroy(&clerk1_mutex);
			pthread_mutex_destroy(&clerk2_mutex);
			pthread_mutex_destroy(&clerk3_mutex);
			pthread_mutex_destroy(&clerk4_mutex);
			pthread_cond_destroy(&clerk1_cond);
			pthread_cond_destroy(&clerk2_cond);
			pthread_cond_destroy(&clerk3_cond);
			pthread_cond_destroy(&clerk4_cond);

			double finishTime = getCurrentSimulationTimeMicro();
			finishTime = finishTime-startUpTime;
			finishTime = convertMicroToSecond(finishTime);
			printf("finish: %2f\n",finishTime);
			double totalE = getTotalWT(0);
			double totalB = getTotalWT(1);
			double total = totalE+totalB;
			double ave = (total/customerCount)/10;
			double eAve = (totalE/E_customer)/10;
			double bAve = (totalB/B_customer)/10;
			printf("The average waiting time for all customer in the system is: %2f seconds.\n",ave);
			printf("The average waiting time for all business-class customers is %2f seconds.\n", bAve);
			printf("The average waiting time for all economy-class customers is %2f seconds. \n",eAve);
			free(WTimeB);
			free(WTimeE);
		}
		else
		{
			printf("Error with thread pointer\n");
		}
		
	}
	else
	{
		printf(" unknonw command: %s\n", argv[0]);
	}
	return 1;
	
}


void* customer_entry(void* cus_info)
{
	int clerk_id;
	struct customer_info* p_myInfo =(struct customer_info*)cus_info;
	struct customer_info c;
	c.user_id = p_myInfo->user_id;
	c.class_type = p_myInfo->class_type;
	c.arrive_Time= p_myInfo->arrive_Time;
	c.service_Time=p_myInfo->service_Time;
	long arriveTime = convertTenthToMicro(c.arrive_Time);
	long serviceTime = convertTenthToMicro(p_myInfo->service_Time);
	int id = c.user_id;
	int class = p_myInfo->class_type; 
	usleep(arriveTime);
	double DArriveTime= getCurrentSimulationTimeMicro();
	printf("A customer arrives: customer ID %d. \n", id);
	void *mutex_cus,*mutex_clerk, *cond_cus, *cond_clerk;
	struct Queue* queue;
	int *status;
	double cur_time;
	int queueId;
	double *WT;
	int *WTind;
	
	if(class==0)
	{
		queue = E_queue;
		mutex_cus = &customer_e_mutex;
		cond_cus = &customer_e_cond;
		mutex_clerk = &clerk_e_mutex;
		cond_clerk = &clerk_e_cond;
		queueId =1;
		status = &E_queue_status;
		WT = WTimeE;
		WTind = &WTimeEind;
	}
	else if(class==1)
	{
		queue = B_queue;
		mutex_cus = &customer_b_mutex;
		cond_cus = &customer_b_cond;
		mutex_clerk = &clerk_b_mutex;
		cond_clerk = &clerk_b_cond;
		queueId =2;
		status = &B_queue_status;
		WT = WTimeB;
		WTind = &WTimeBind;
	}
	else
	{
		printf("Error: no class for customer!\n");
		return (void*)0;
	}
	if(!isFull(queue))
	{
		pthread_mutex_lock(mutex_cus);
		enqueue(queue,c);


		fprintf(stdout,"A customer enters a queue: the queue ID %1d, and length of queue %2d. \n",queueId,queue->size);
		pthread_cond_wait(cond_cus,mutex_cus);
		while(front(queue).user_id!=id || *status ==-1)
		{
			pthread_cond_wait(cond_cus,mutex_cus);
		}
		cur_time = getCurrentSimulationTimeMicro();
		double ser_time = cur_time- startUpTime;
		ser_time = ser_time/100000;
		clerk_id = *status;
		dequeue(queue);
		*status =-1;
		pthread_mutex_unlock(mutex_cus);
		printf("A clerk starts serving a customer: start time %2f, the customer ID %2d, the clerk ID %1d. \n",ser_time,id,clerk_id);
		usleep(serviceTime);

		if(clerk_id==1)
		{
			pthread_cond_signal(&clerk1_cond);
		}
		if(clerk_id==2)
		{
			pthread_cond_signal(&clerk2_cond);
		}
		if(clerk_id==3)
		{
			pthread_cond_signal(&clerk3_cond);
		}
		if(clerk_id==4)
		{
			pthread_cond_signal(&clerk4_cond);
		}
	}
	else
	{
		printf("Error: Queue %d is full!\n",queueId);
		return (void*)0;
	}
	double waitingTime = (cur_time-DArriveTime)/100000;
	printf("waiting Time: %2f \n", waitingTime);
	WT[*WTind] = waitingTime;
	*WTind = *WTind+1;
	double te = getCurrentSimulationTimeMicro();
	te =te -startUpTime;
	te=(te/1e+6)*10;
	fprintf(stdout, "A clerk finishes serving a customer: end time %2f, the customer ID %d, the clerk ID %1d. \n",te,id,clerk_id );
	
	return (void*)0;
}
void* clerk_entry(void* clerkNum)
{
	int id = *((int*)clerkNum);
	int printed =1;
	while(finish ==0)
	{
		if(isEmpty(B_queue)&&isEmpty(E_queue))
		{
			if(!printed)
			{
				printed =1;
				printf("Clerk %d is idle\n", id);
			}
			usleep(100000);
			continue;
		}
		struct Queue* queue;
		void *cond_cus,*cond_clerk,*mutex_cus,*mutex_clerk;
		int* status;
		if(!isEmpty(B_queue))
		{
			queue = B_queue;
			cond_cus = &customer_b_cond;
			cond_clerk = &clerk_b_cond;
			mutex_cus = &customer_b_mutex;
			mutex_clerk = &clerk_b_mutex;
			status= &B_queue_status;
		}
		else if(!isEmpty(E_queue))
		{
			queue = E_queue;
			cond_cus = &customer_e_cond;
			cond_clerk = &clerk_e_cond;
			mutex_cus = &customer_e_mutex;
			mutex_clerk = &clerk_e_mutex;
			status= &E_queue_status;
		}
		if(*status!=-1)
		{
			usleep(100000);
			continue;
		}
		if(isEmpty(queue))
		{
			continue;
		}
		int ret = pthread_mutex_trylock(mutex_clerk);
		if(ret!=0)
		{
			continue;
		}
		*status = id;
		int cus = front(queue).user_id;
		pthread_cond_broadcast(cond_cus);
		while(front(queue).user_id==cus)
		{
			usleep(1000);
		}
		pthread_mutex_unlock(mutex_clerk);
		void* clerkID_mutex;
		void* clerkID_cond;
		if(id ==1)
		{
			clerkID_mutex = &clerk1_mutex;
			clerkID_cond = &clerk1_cond;
		}
		else if(id ==2)
		{
			clerkID_mutex = &clerk2_mutex;
			clerkID_cond = &clerk2_cond;
		}
		else if(id ==3)
		{
			clerkID_mutex = &clerk3_mutex;
			clerkID_cond = &clerk3_cond;
		}
		else if(id ==4)
		{
			clerkID_mutex = &clerk4_mutex;
			clerkID_cond = &clerk4_cond;
		}
		pthread_cond_wait(clerkID_cond,clerkID_mutex);
		pthread_mutex_unlock(clerkID_mutex);
		printed = 0;	
	}
	return (void*)1;
}
double getTotalWT(int class)
{
	double* WTs;
	int max_ind = 0;
	if(class==0)
	{
		WTs = WTimeE;
		max_ind = WTimeEind;
	}
	else if(class==1)
	{
		WTs = WTimeB;
		max_ind = WTimeBind;
	}
	int i;
	double total = 0;
	for(i =0;i<max_ind;i++)
	{
		total = total +WTs[i];
	}
	return total;
}
