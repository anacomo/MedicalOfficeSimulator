#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define QUEUE_EMPTY INT_MIN

typedef struct node
{
    int doctor_id;
    struct node *next;
} node;

typedef struct
{
    node *head;
    node *tail;
} queue;


struct patient
{
    pthread_t my_thread;
    int patient_id;
    time_t arrival_time;
    time_t burst_time;
    time_t completion_time;
    time_t turnaround_time;
    time_t waiting_time;
    int nr_consultations;
    queue arrival_times_queue;
    queue burst_times_queue;
};

struct doctor
{
    pthread_mutex_t mutex;
    int id;
    int available;
};

const int doctor_numbers = 3;
struct doctor doctors[3];

const int nr_patients = 5;
struct patient patients[5];

int finished_patients = 0;

sem_t semaphore;
pthread_mutex_t waiting_room;
pthread_mutex_t resend;

FILE * file;