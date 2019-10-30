//
// Created by Nicholas Porrone on 2019-10-23.
//

#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// A structure to represent a Process
struct Process
{
    int processId;
    int burstTime;
    int waitTime;
    int turnaroundTime;
};

// function to create a process

struct Process* createProcess(int pid, int bTime)
{
    struct Process* currentProcess = (struct Process*) malloc(sizeof(struct Process));
    currentProcess->processId = pid;
    currentProcess->burstTime = bTime;
    currentProcess->waitTime = 0;
    currentProcess->turnaroundTime = 0;
    return currentProcess;
}

// A structure to represent a queue
struct Queue
{
    int front, rear, size, qtime, id;
    unsigned capacity;
    struct Process* array[100];
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity, int qtime, int OrderID)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity; // capacity of the queue
    queue->qtime = qtime; // Quantum time used in RR
    queue->id = OrderID; // Id for the queue in the file
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    for(int i = 0; i<queue->capacity;i++){
        queue->array[i] = malloc(sizeof(struct Process));
    }
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{  return (queue->size == queue->capacity);  }

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{  return (queue->size == 0); }

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct Process *item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;

}

// Function to remove an item from queue.
// It changes front and size
struct Process* dequeue(struct Queue* queue)
{
    struct Process* item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

void fcfs(struct Queue *current_queue, int queue_count);  // First come first serve

void sjf(struct Queue *current_queue, int queue_count); // Shortest Job First

void rr(struct Queue *current_queue, int queue_count); // Round Robin

void printFCFS(struct Process *pProcess[100], int pcount, int qcount, int avgwait); // helper used to print FCFS

void printSJF(struct Process *pProcess[100], int pcount, int qcount, int avgwait); // helper used to print SJF

void printRR(struct Process *pProcess[100], struct Process *orderToPrint[100], int pcount, int qcount); // helper used to print RR

int main(int argc, char **argv) {
    // Check the correct amount of arguments are given
    if (argc != 2) {
        printf("Invalid Arguments, Please pass in the name of the CPU scheduling txt file.");
    }
    FILE *file; // store the file in this variable
    size_t bufsize = 256;  // the size needed for the buffer
    char* buf = malloc(sizeof(char)*bufsize); // Buffer needed to read the file
    file = fopen(argv[1], "r");

    struct Queue *queueHolder[100]; // Queue to hold the Queues (guessed a limit of 100 queues)

    for(int i = 0; i<100;i++){
        queueHolder[i] = malloc(sizeof(struct Queue));
    }

    if (file) { // if file is open

        while ((getline(&buf,&bufsize,file)) != EOF) {  // read each line until the end of file is reached
            struct Queue *ReadyQueue; // Ready Queue used to store the processes
            int queueOrder, qtime_queue; // needed to create a ready queue
            char* token = strtok(buf, " "); // holds each part of the line

            while (token != NULL) {

                if (strcmp(token,"q") == 0) {

                    queueOrder = atoi(strtok(NULL, " "));  // Store which Queue it is
                    strtok(NULL, " ");  // Move to tq
                    qtime_queue = atoi(strtok(NULL, " "));  // Move to Quantum time used in RR for the queue

                    ReadyQueue = createQueue(15,qtime_queue,queueOrder);
                    queueHolder[queueOrder - 1] = ReadyQueue;

                    token = strtok(NULL, " ");  // Move to first process

                } else {
                    int processName = atoi(token + 1);  // Move to process name ex. p1
                    char* btime = strtok(NULL, " ");  // Move to burst time for the process
                    struct Process *current_process = createProcess(processName, atoi(btime));  // Create Process

                    enqueue(ReadyQueue,current_process);

                    token = strtok(NULL, " ");  // Move to next process
                }

            }

        }
        fclose(file);

        // Call CPU Scheduling algorithms

        int queue_count = 1; // Used for printing

        for(int i = 0; i < 100; i++) { // while the array holding all ready queues is not empty
            fcfs(queueHolder[i],queue_count);
            sjf(queueHolder[i],queue_count);
            rr(queueHolder[i],queue_count);
            queue_count += 1;
        }

    }



}

void fcfs(struct Queue *current_queue, int queue_count) {

    struct Process *completed[100];

    int timeElapsed = 0; // running total on the time count
    int totalWaitTime = 0; // running total on the wait time - used to calculate avg wait time.
    int averageWaitTime = 0; // Important measuring tool
    int process_count = 0; // counter for the process'

    while (!isEmpty(current_queue)){  // while the ready queue is not empty

        struct Process *current_process = dequeue(current_queue); // current process

        // update all attributes
        current_process->waitTime = timeElapsed;
        current_process->turnaroundTime = current_process->burstTime;
        totalWaitTime += current_process->waitTime;
        timeElapsed += current_process->burstTime;

        completed[process_count] = current_process;
        process_count += 1;

    }

    if (process_count > 0 ) { // check if queue is full
        averageWaitTime = totalWaitTime/process_count; // compute average wait time
        printFCFS(completed,process_count,queue_count,averageWaitTime);

        // Fill up the Queue again so it can still be used again
        for(int i = 0; i < process_count; i++){
            enqueue(current_queue,completed[i]);
        }
    }
}

// Shortest Job First

void sjf(struct Queue *current_queue, int queue_count) {

    struct Process *save[100];
    struct Process *completed[100];
    struct Process *completedSorted[100];

    int timeElapsed = 0; // running total on the time count
    int totalWaitTime = 0; // running total on the wait time - used to calculate avg wait time.
    int averageWaitTime = 0; // Important measuring tool
    int process_count = 0; // counter for the process'

    while (!isEmpty(current_queue)){  // while the ready queue is not empty

        struct Process *current_process = dequeue(current_queue); // current process

        completed[process_count] = current_process;
        process_count += 1;

    }

    // save completed so you can fill the queue later for other scheduling
    for(int i = 0; i < process_count ; i++) {
        save[i] = completed[i];
    }


    // sort completed
    int min[2]; // tuple to store id & burst time

    for(int i = 0; i < process_count; i++) { // iterate through sorted

        min[0] = 99999; // Min burst time
        min[1] = 0; // ID
        for (int j = 0; j < process_count; j++) {
            if (completed[j]->burstTime < min[0]) {
                min[0] = completed[j]->burstTime;
                min[1] = j;
            }
        }
        completedSorted[i] = completed[min[1]];
        struct Process *clear = createProcess(i+2000,9999);
        completed[min[1]] = clear; // take out
    }

    for(int i = 0; i < process_count; i++){
        // update all attributes
        completedSorted[i]->waitTime = timeElapsed;
        completedSorted[i]->turnaroundTime = completedSorted[i]->burstTime;
        totalWaitTime += completedSorted[i]->waitTime;
        timeElapsed += completedSorted[i]->burstTime;
    }

    if (process_count > 0 ) { // check if queue is full
        averageWaitTime = totalWaitTime/process_count; // compute average wait time
        printSJF(completedSorted,process_count,queue_count,averageWaitTime);

        // Fill up the Queue again so it can still be used again
        for(int i = 0; i < process_count; i++){
            enqueue(current_queue,save[i]);
        }
    }
}

void rr(struct Queue *current_queue, int queue_count) {

    struct Process *completed[100]; // final product
    struct Process *save[100]; // used to save the queue

    int timeElapsed = 0; // running total on the time count
    int totalWaitTime = 0; // running total on the wait time - used to calculate avg wait time.
    int process_count = 0; // counter for the process'

    while (!isEmpty(current_queue)) {  // while the ready queue is not empty

        struct Process *current_process = dequeue(current_queue); // current process

        save[process_count] = current_process;     // save completed so you can fill the queue later for other scheduling
        process_count += 1;

    }

    // Create a queue you can manipulate
    // Round Robin
    struct Queue *completedRR = createQueue(process_count,current_queue->qtime,1000);
    for (int i = 0; i < process_count; i++){
        save[i]->turnaroundTime = 0; // clear turnaround time
        enqueue(completedRR,save[i]);
    }

    // Round 1 of RR
    // count how many processes are completed
    int counter = 0;
    for(int i = 0; i < process_count; i++) {

        struct Process *current_process = dequeue(completedRR); // current process
        current_process->waitTime = timeElapsed;

        if (current_process->burstTime <= completedRR->qtime) { // if burst is less than or equal to q
            // update all attributes
            current_process->turnaroundTime = current_process->burstTime;
            timeElapsed += current_process->burstTime;
            completed[counter] = current_process;
            counter += 1; // increment counter

        } else {
            current_process->burstTime -= completedRR->qtime; // update burst time
            timeElapsed += completedRR->qtime;
            enqueue(completedRR, current_process);  // send it to the back of the queue
        }

    }

    // Rounds 2 and on of RR

    while (!isEmpty(completedRR)) {  // while the ready queue is not empty

        struct Process *current_process = dequeue(completedRR); // current process

        if (current_process->burstTime <= completedRR->qtime){ // if burst is less than or equal to q
            // update all attributes
            current_process->turnaroundTime = current_process->burstTime + timeElapsed - current_process->waitTime;
            timeElapsed += current_process->burstTime;
            completed[counter] = current_process;
            counter += 1; // increment counter

        } else {
            current_process->burstTime -= completedRR->qtime; // update burst time
            timeElapsed += completedRR->qtime;
            enqueue(completedRR,current_process);  // send it to the back of the queue
        }

    }


    if (process_count > 0 ) { // check if queue is full
        printRR(completed,save,process_count,queue_count);

        // Fill up the Queue again so it can still be used again
        for(int i = 0; i < process_count; i++){
            enqueue(current_queue,save[i]);
        }
    }
}




// Helper function used to print the schedule of a queue for FCFS

void printFCFS(struct Process *processToPrint[100], int pcount, int qcount, int avgwait) {
    printf("---------------------------------------------------------------------------------------------\n");
    printf("Ready Queue %d Applying FCFS Scheduling: \n",qcount);
    printf("\nOrder of selection by CPU: \n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d ", processToPrint[i]->processId);
    }

    printf("\n\nIndividual waiting times for each process:\n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d = %d\n", processToPrint[i]->processId, processToPrint[i]->waitTime);
    }

    printf("\nAverage waiting time = %d\n", avgwait);
}

// Helper function used to print the schedule of a queue for SJB

void printSJF(struct Process *processToPrint[100], int pcount, int qcount, int avgwait) {
    printf("---------------------------------------------------------------------------------------------\n");
    printf("Ready Queue %d Applying SJF Scheduling: \n",qcount);
    printf("\nOrder of selection by CPU: \n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d ", processToPrint[i]->processId);
    }

    printf("\n\nIndividual waiting times for each process:\n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d = %d\n", processToPrint[i]->processId, processToPrint[i]->waitTime);
    }

    printf("\nAverage waiting time = %d\n", avgwait);
}

void printRR(struct Process *processToPrint[100],struct Process *processOrder[100], int pcount, int qcount) {
    printf("---------------------------------------------------------------------------------------------\n");
    printf("Ready Queue %d Applying RR Scheduling: \n",qcount);
    printf("\nOrder of selection by CPU: \n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d ", processOrder[i]->processId);
    }

    printf("\n\nIndividual Turnaround times for each process:\n");
    for (int i = 0; i < pcount; i++) {
        printf("p%d = %d\n", processToPrint[i]->processId, processToPrint[i]->turnaroundTime);
    }

}









