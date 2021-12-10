#include<stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LEN 1000 //max length for input lines
#define MAX_CHAR 80 //max length for output lines
#define MAX_LINES 49 //max amount of lines to output

//thread signals
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readT = PTHREAD_COND_INITIALIZER;
pthread_cond_t lineT = PTHREAD_COND_INITIALIZER;
pthread_cond_t plusT = PTHREAD_COND_INITIALIZER;
pthread_cond_t outT = PTHREAD_COND_INITIALIZER;

//thread processes
int t1 = 1; //default as 1 so program can start with the read thread
int t2 = 0; //default as 0 so it pauses until signal by its producer
int t3 = 0;
int t4 = 0;

int lines = 0; //keep track number of lines output
int new_input = 0; //loop condition on whether the thread should read next input line
char input[MAX_LEN]; //input with length 1000
char output[MAX_CHAR]; //output with length 80
int input_index = 0; //keep track current character of input
int output_index = 0; //keep track current character of output

void *thread1(void *args)
{
    while (1) //keeps thread running until STOP is received or max output lines reached
    {
        fgets(input, MAX_LEN, stdin);
        new_input = 0;
        input_index = 0;
        while (!new_input) //loop through one single input line
        {
            pthread_mutex_lock(&mutex);
            while (t1 == 0) //wait for signal
            {
                pthread_cond_wait(&readT, &mutex);
            }
            //terminate if STOP is received
            if (input[0] == 'S' && input[1] == 'T' && input[2] == 'O' && input[3] == 'P' && input[4] == '\n')
            {
                exit(0);
            }

            while (output_index < MAX_CHAR) //loop through input until output is full
            {
                if (input[input_index] >= 32 && input[input_index] <= 126) //only accept characters between decimal 32 and 126
                {
                    output[output_index] = input[input_index]; //copy input character to output
                    input_index++;
                    output_index++;
                }
                else
                {
                    new_input = 1; //new line detected, end loop
                    if (input[input_index - 1] != 32) //if character before newline is not space
                    {                                 //add space instead of newline
                        output[output_index] = 32;
                        input_index++;
                        output_index++;
                    }
                    break;
                }
            }
            t1 = 0; //pauses thread 1
            t2 = 1; //starts thread 2
            pthread_cond_signal(&lineT);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

void *thread2(void *args)
{
    while (1) //keeps thread running until STOP is received or max output lines reached
    {
        pthread_mutex_lock(&mutex);
        while (t2 == 0) //wait for signal
        {
            pthread_cond_wait(&lineT, &mutex);
        }
        for (int i = 0; i < output_index; i++) //loop through output, convert any newline to space
        {
            if (output[i] == '\n')
            {
                output[i] = 32;
            }
            i++;
        }
        t2 = 0; //pauses thread 2
        t3 = 1; //starts thread 3
        pthread_cond_signal(&plusT);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *thread3(void *args)
{
    while (1) //keeps thread running until STOP is received or max output lines reached
    {
        pthread_mutex_lock(&mutex);
        while (t3 == 0) //wait for signal
        {
            pthread_cond_wait(&plusT, &mutex);
        }
        int j = 0;
        if (output_index == MAX_CHAR)
        {
            while (output[j + 1]) //loop through output 2 characters at a time
            {
                if (output[j] == '+' && output[j + 1] == '+') //if two consecutive ++, convert to ^
                {
                    output[j] = '^';
                    int k;
                    k = j + 1;
                    while (output[k]) //fill the second + by moving every character back by one
                    {
                        output[k] = output[k + 1];
                        k++;
                    }
                    output_index--;
                }
                j++;
            }
        }
        t3 = 0; //pauses thread 3
        t4 = 1; //starts thread 4
        pthread_cond_signal(&outT);
        pthread_mutex_unlock(&mutex);
    }
}

void *thread4(void *args)
{
    while (1) //keeps thread running until STOP is received or max output lines reached
    {
        pthread_mutex_lock(&mutex);
        while (t4 == 0) //wait for signal
        {
            pthread_cond_wait(&outT, &mutex);
        }
        if (output_index == MAX_CHAR) //output if full
        {
            printf("%s\n", output);
            output_index = 0;
            lines++;
        }
        if (lines == MAX_LINES) //terminate if max lines reached
        {
            exit(0);
        }
        t4 = 0; //pauses thread 4
        t1 = 1; //starts thread 1
        pthread_cond_signal(&readT);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    //initialize, create, and join threads
    pthread_t r, l, p, o;
    pthread_create(&r, NULL, thread1, NULL);
    pthread_create(&l, NULL, thread2, NULL);
    pthread_create(&p, NULL, thread3, NULL);
    pthread_create(&o, NULL, thread4, NULL);
    pthread_join(r, NULL);
    pthread_join(l, NULL);
    pthread_join(p, NULL);
    pthread_join(o, NULL);
    
    return 0;
}