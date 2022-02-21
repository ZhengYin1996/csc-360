#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"

struct customer {
    int id;
    int class;
    int arrival_time;
    int service_time;
};

void *customer_thread_function(void *param);
void *clerk_thread_function(void *param);
long getCurrentTimeMicro();
long convertTenthToMicro(long tenth);
long convertMicroToTenth(long micro);
int getTotalWaitingTimes(int class);

pthread_mutex_t B_CUSTOMER_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t B_CLERK_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t B_CUSTOMER_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t B_CLERK_cv = PTHREAD_COND_INITIALIZER;

pthread_mutex_t E_CUSTOMER_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t E_CLERK_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t E_CUSTOMER_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t E_CLERK_cv = PTHREAD_COND_INITIALIZER;

pthread_mutex_t clerk1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clerk4_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk1_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk2_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk3_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk4_cv = PTHREAD_COND_INITIALIZER;

struct Queue* q_e;
struct Queue* q_b;
int queue_e_status = -1;
int queue_b_status = -1;
int amount_customers;
int finished;
long startUpTime = 0;
int *waitingTimesB = NULL;
int *waitingTimesE = NULL;
int waitingTimesBind = 0;
int waitingTimesEind = 0;

int main(int argc, char* argv[])
{
	if (argc == 2){
        finished = 0;
        amount_customers = 0;
        // Read the customers.txt data file and create the customer strucs
        FILE *fp = fopen(argv[1], "r");
        if(fp == NULL){
            printf("Error reading file %s\n",argv[1]);
            return 0;
        } 
        char line[100];
        // Read the first line of the file, which is the amount of customers
        fgets(line, sizeof(line), fp);
        amount_customers = atoi(line);
        printf("Amount of customers: %d\n",amount_customers);
        pthread_t threads[amount_customers];
        struct customer customers[amount_customers];
        int count_B = 0;
        int count_E = 0;
        int i = 0;
        while(fgets(line, sizeof(line), fp)){
            char* token = strtok(line, ":");
            int id = atoi(token);
            token = strtok(0, ",");
            int class = atoi(token);
            token = strtok(0, ",");
            int arrival_time = atoi(token);
            token = strtok(0, ",");
            int service_time = atoi(token);
            //printf("id: %d class: %d arrival_time: %d service_time: %d\n",id,class,arrival_time,service_time);
            customers[i].id = id;
            customers[i].class = class;
            customers[i].arrival_time = arrival_time;
            customers[i].service_time = service_time;
            i = i + 1;
            if(class == 0)  count_E = count_E + 1;
            if(class == 1)  count_B = count_B + 1;
        }            
        fclose(fp);

        waitingTimesB = (int *) malloc(sizeof(int) * count_B);
        waitingTimesE = (int *) malloc(sizeof(int) * count_E);
        pthread_t clerk1_thread;
        pthread_t clerk2_thread;
        pthread_t clerk3_thread;
        pthread_t clerk4_thread;
        int clerk1 = 1, clerk2 = 2, clerk3 = 3, clerk4 = 4;
        q_e = createQueue(count_E);
        q_b = createQueue(count_B);
        if(pthread_create(&clerk1_thread, NULL, clerk_thread_function, &clerk1) != 0)   fprintf(stderr, "error: Cannot create clerk thread.\n");
        if(pthread_create(&clerk2_thread, NULL, clerk_thread_function, &clerk2) != 0)   fprintf(stderr, "error: Cannot create clerk thread.\n");
        if(pthread_create(&clerk3_thread, NULL, clerk_thread_function, &clerk3) != 0)   fprintf(stderr, "error: Cannot create clerk thread.\n");
        if(pthread_create(&clerk4_thread, NULL, clerk_thread_function, &clerk4) != 0)   fprintf(stderr, "error: Cannot create clerk thread.\n");               
        for(i = 0; i < amount_customers; i++){
            struct customer *c = &customers[i];
            if(pthread_create(&threads[i], NULL, customer_thread_function, c) != 0){
                fprintf(stderr, "error: Cannot create thread # %d\n", i);
                return 0;
            }
        }
        startUpTime = getCurrentTimeMicro();
        if(threads){
            int b;
            for(b = 0; b < amount_customers; b = b + 1){
                pthread_join(threads[b],NULL);
            }
            // Once all customer threads return, we are done.
            finished = 1;
            pthread_mutex_destroy(&E_CUSTOMER_mutex);
            pthread_mutex_destroy(&E_CLERK_mutex);
            pthread_mutex_destroy(&B_CUSTOMER_mutex);
            pthread_mutex_destroy(&B_CLERK_mutex);
	        pthread_cond_destroy(&E_CUSTOMER_cv);
            pthread_cond_destroy(&E_CLERK_cv);
            pthread_cond_destroy(&B_CUSTOMER_cv);
            pthread_cond_destroy(&B_CLERK_cv);
            pthread_mutex_destroy(&clerk1_mutex);
            pthread_mutex_destroy(&clerk2_mutex);
            pthread_mutex_destroy(&clerk3_mutex);
            pthread_mutex_destroy(&clerk4_mutex);
	        pthread_cond_destroy(&clerk1_cv);
            pthread_cond_destroy(&clerk2_cv);
            pthread_cond_destroy(&clerk3_cv);
            pthread_cond_destroy(&clerk4_cv);
            printf("Finished serving all customers.\n");
            double finishTime = (double)getCurrentTimeMicro();
            finishTime = finishTime - startUpTime;
            finishTime = (double)convertMicroToTenth(finishTime);
            finishTime = finishTime / 10;
            double totalE = (double)getTotalWaitingTimes(0);
            double totalB = (double)getTotalWaitingTimes(1);
            double total = totalE+totalB;
            double totalAvg = (total/amount_customers) / 10;
            double eAvg = (totalE / count_E) / 10;
            double bAvg = (totalB / count_B) / 10;
            printf("The average waiting time for all customers in the system is: %.2f seconds.\n",totalAvg);
            printf("The average waiting time for all business-class customers is: %.2f seconds.\n",bAvg);
            printf("The average waiting time for all economy-class customers is: %.2f seconds.\n",eAvg);
            printf("Finished in %.2f seconds.\n",finishTime);
            free(waitingTimesB);
            free(waitingTimesE);
        }else{  printf("Error with thread array pointer.\n");   }
    } else {    printf("Invalid command. Usage: %s <filename>.txt\n",argv[0]);  }
}
 
/**
 * customer_thread_function, for use by each customer thread.
 * Parameter is the customer struct which contains the customer data
 * */
void* customer_thread_function(void *param){
    int clerk_id;
    struct customer *c = param;
    long arrival_time = c->arrival_time;
    arrival_time = convertTenthToMicro(arrival_time);
    long service_time = c->service_time;
    service_time = convertTenthToMicro(service_time);
    long seconds = arrival_time/1000000;
    int id = c->id;
    int class = c->class;
    printf("Customer %d arriving in %ld seconds\n",id, seconds);
    usleep(arrival_time); // Wait for arrival_time microseconds
    printf("A customer arrives: %d\n",id);
    long arrivalTime = convertMicroToTenth(getCurrentTimeMicro() - startUpTime);
    long ts;
    struct Queue* q;
    void *mutex_customer, *mutex_clerk, *cond_customer, *cond_clerk;
    int queue_id;
    int *status;
    int *waitingTimes;
    int *waitingTimesind;
    // Define which mutexes and condition variables to use based on the class.
    if(class == 0){
        q = q_e;
        mutex_customer = &E_CUSTOMER_mutex;
        mutex_clerk = &E_CLERK_mutex;
        cond_customer = &E_CUSTOMER_cv;
        cond_clerk = &E_CLERK_cv;
        queue_id = 1;
        status = &queue_e_status;
        waitingTimes = waitingTimesE;
        waitingTimesind = &waitingTimesEind;
    }
    else if(class == 1){
        q = q_b;
        mutex_customer = &B_CUSTOMER_mutex;
        mutex_clerk = &B_CLERK_mutex;
        cond_customer = &B_CUSTOMER_cv;
        cond_clerk = &B_CLERK_cv;
        queue_id = 0;
        status = &queue_b_status;
        waitingTimes = waitingTimesB;
        waitingTimesind = &waitingTimesBind;
    }else{
        printf("[Error] Class of customer %d is neither 1 or 0: %d\n",id,class);
        return (void *)0;
    }
    if(!isFull(q)){
        pthread_mutex_lock(mutex_customer);
        enqueue(q,id);
        printf("Customer %d enters queue %d of length %d\n",id,queue_id,q->size);
        pthread_cond_wait(cond_customer, mutex_customer); //relase mutex lock
        while(front(q) != id || *status == -1){
            pthread_cond_wait(cond_customer, mutex_customer);
        }
        ts = getCurrentTimeMicro();
        ts = ts - startUpTime;
        ts = convertMicroToTenth(ts);
        clerk_id = *status; // Retrieve the clerk id
        dequeue(q);
        *status = -1;
        pthread_mutex_unlock(mutex_customer); // Unlock the mutex for another customer to join the queue / get serviced by a clerk
        printf("A clerk starts serving a customer: Start time: %ld, Customer ID: %d, Clerk ID %d.\n",ts,id,clerk_id);
        usleep(service_time); // Get serviced for service_time microseconds
        // Signal the clerk that was serving us to show that we are done
        if(clerk_id == 1)   pthread_cond_signal(&clerk1_cv);
        if(clerk_id == 2)   pthread_cond_signal(&clerk2_cv);
        if(clerk_id == 3)   pthread_cond_signal(&clerk3_cv);
        if(clerk_id == 4)   pthread_cond_signal(&clerk4_cv);
        if(clerk_id == -1)  printf("[ERROR] Customer %d served by -1\n",id);
    }else{
        printf("[Error] Queue %d full.\n",class);
        return (void *)0;
    }
    long waitingTime = ts - arrivalTime;
    waitingTimes[*waitingTimesind] = waitingTime; // Store the waiting time for this customer in the proper waiting time array
    *waitingTimesind = *waitingTimesind + 1;
    long te = getCurrentTimeMicro();
    te = te - startUpTime;
    te = convertMicroToTenth(te);
    printf("A clerk finishes serving a customer: End time: %ld, Customer ID: %d, Clerk ID %d.\n",te,id,clerk_id);
    return (void *) 1;
}

/**
 * clerk_thread_function, for use by each clerk thread.
 * Parameter is the id of the clerk
 * */
void* clerk_thread_function(void *param){
    int id = *((int *)param);
    printf("Clerk %d idle.\n",id);
    int printed = 1;
    while(finished == 0){
        if(isEmpty(q_b) && isEmpty(q_e)){
            if(!printed){
                printed = 1;
                printf("Clerk %d idle.\n",id);
            }
            usleep(100000);
            continue;
        }
        struct Queue* q;
        void *cond_customer, *cond_clerk, *mutex_customer, *mutex_clerk;
        int* status;       
        if(!isEmpty(q_b)){ // If the business queue is not empty, we must serve those customers first
            q = q_b;
            cond_customer = &B_CUSTOMER_cv;
            cond_clerk = &B_CLERK_cv;
            mutex_customer = &B_CUSTOMER_mutex;
            mutex_clerk = &B_CLERK_mutex;
            status = &queue_b_status;            
        }
        else if(isEmpty(q_b) && !isEmpty(q_e)){
            q = q_e;
            cond_customer = &E_CUSTOMER_cv;
            cond_clerk = &E_CLERK_cv;
            mutex_customer = &E_CUSTOMER_mutex;
            mutex_clerk = &E_CLERK_mutex;
            status = &queue_e_status;
        }
        if(*status != -1){
            usleep(10000);
            continue;
        }
        if(isEmpty(q)) continue;
        int ret = pthread_mutex_trylock(mutex_clerk); // Check to see if this queue is being used by another clerk.
        if(ret != 0)   continue; // A clerk is already using that queue, try a different queue
        *status = id; // Set the queue status to the clerk id for the customer to retrieve
        int customer = front(q);
        pthread_cond_broadcast(cond_customer); // Broadcast to all customers in the queue
        while(front(q) == customer){ // Wait for the customer at the front to store the clerk id and dequeue themselves
            usleep(1000);
        }
        pthread_mutex_unlock(mutex_clerk); // Unlock the clerk mutex for the queue so another clerk may choose the next customer
        // Wait on the correct condition variable. When the customer is done getting serviced they will signal this variable
        void *clerk_ID_mutex;
        void *clerk_ID_cv;   
        if(id == 1){
            clerk_ID_mutex = &clerk1_mutex;
            clerk_ID_cv = &clerk1_cv;
        }
        if(id == 2){
            clerk_ID_mutex = &clerk2_mutex;
            clerk_ID_cv = &clerk2_cv;
        }
        else if(id == 3){
            clerk_ID_mutex = &clerk3_mutex;
            clerk_ID_cv = &clerk3_cv;
        }
        else if(id == 4){
            clerk_ID_mutex = &clerk4_mutex;
            clerk_ID_cv = &clerk4_cv;
        }
        pthread_cond_wait(clerk_ID_cv, clerk_ID_mutex);
        pthread_mutex_unlock(clerk_ID_mutex);
        printed = 0; // We have finished serving a customer, reset the debug boolean for when we go back to the idle state.
    }
    return (void *) 1;
}

// Get current system time in microseconds
long getCurrentTimeMicro(){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    long delta_us = (spec.tv_sec * 1000000) + (spec.tv_nsec / 1000);
    return delta_us;
}

// Convert microseconds to tenth of a seconds
long convertMicroToTenth(long micro){
    return (micro/1e+6)*10;
}
// Convert tenth of a seconds to microseconds
long convertTenthToMicro(long tenth){
    return (tenth/10)*1e+6;
}


// Get total waiting times for a particular class
int getTotalWaitingTimes(int class){
    int *waitingTimes;
    int max_ind = 0;
    if(class == 0){
        waitingTimes = waitingTimesE;
        max_ind = waitingTimesEind;
    }
    if(class == 1){
        waitingTimes = waitingTimesB;
        max_ind = waitingTimesBind;
    }
    int j;
    int total = 0;
    for(j = 0; j < max_ind; j++){
        total = total + waitingTimes[j];
    }
    return total;
}