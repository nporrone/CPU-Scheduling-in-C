//
// Created by nporr on 2019-10-23.
//

#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// Variables to add

int x = 10;
int y = 20;
int z = 0;

// Thread Function to sum both integers

void *thread_sums_two_integers () {

    z = x + y;

    printf("From Thread: Calculation was complete\n");

}

int main() {
    pid_t pid; // create a process identifier used for the fork call
    pthread_t thread; // used to create a thread (name of thread)
    pid=fork(); // create the first child

    if (pid < 0 ) {  //unsuccessful fork
        printf("fork unsuccessful");
    }
    if (pid > 0) {  // parent
        wait(NULL); // wait for child created by fork to add the integers
        printf("The value of z is %d\n",z);
        printf("Lets try a thread:\n");
        pthread_create(&thread, NULL, thread_sums_two_integers, NULL);
        pthread_join(thread,NULL);
        printf("The value of z is %d\n",z);
    }
    if (pid == 0 ) {  // child
        z = x + y;

    }
    return 0; // end the process
}