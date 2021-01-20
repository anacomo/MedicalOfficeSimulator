# Oncology Medical Office Simulator

## Author: Ana-Maria Comorasu

Task: Implement a program which simulates a medical office: there will be created a number of threads (which represent the patients) which will wait for a cabinet to be vacated, represented by the doctors. The consultation will consist of blocking the doctor structure for the burst time.

* Generating input  
[generator.c](https://github.com/anacomo/MedicalOfficeSimulator/blob/main/generator.c) will generate content for [generate.txt](https://github.com/anacomo/MedicalOfficeSimulator/blob/main/generate.txt)  

* Data structures and components
1. Patient
``` C
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

const int nr_patients = 5;
struct patient patients[5];
```

2. Doctor
``` C
struct doctor
{
    pthread_mutex_t mutex;
    int id;
    int available;
};

const int doctor_numbers = 3;
struct doctor doctors[3];
```

3. Queue
```C
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
```


* Starting the thread pool

``` C
// threads
for (int i = 0; i < nr_patients; ++i)
{
    int *count = malloc(sizeof count);
    *count = i;
    if (pthread_create(&patients[i].my_thread, NULL, send_to_consultation, count))
    {
        perror("Error trying to create threads");
        return errno;
    }
    if (i <= nr_patients - 2)
    {
        sleep(patients[i + 1].arrival_time - patients[i].arrival_time);
    }
}
```

* Send to consultation  
This function reuses the thread (patient) if he has another consultation scheduled
```C
void *send_to_consultation(void *arg)
{
    int patient = *((int *)arg);
    waiting(patient, 0);
    while (isAnyDoctorAvailable(&patients[patient].arrival_times_queue))
    {
        pthread_mutex_lock(&resend);
        patients[patient].arrival_time = dequeue(&patients[patient].arrival_times_queue);
        patients[patient].burst_time = dequeue(&patients[patient].burst_times_queue);
        time_t late_time = difftime(patients[patient].completion_time, patients[patient].arrival_time);
        if (patients[patient].completion_time > patients[patient].arrival_time)
        {
            pthread_mutex_unlock(&resend);
        }
        else
        {
            pthread_mutex_unlock(&resend);
            sleep(-late_time);
        }
        if(late_time < 0)
        {
            late_time = 0;
        }
        waiting(patient, late_time);
    }
    return NULL;
}
```

* Waiting room function  
The patient waits at the semaphore, which is first initialized with the number of doctors available.  
Also, the function calculates the total waiting time and sends the patient to the consultation.  
After the consultation, the thread will post at the semaphore, which means that a an office is now free.  
The doctors' ids are retained in a queue, so the patient will go to the first doctor added to the queue.

```C
void waiting(int patient, time_t late_time)
{
    time_t start_time, stop_time, total_time;

    time(&start_time);
    sem_wait(&semaphore);
    time(&stop_time);

    pthread_mutex_lock(&waiting_room);
    int doctor_id = dequeue(&available_doctors);
    total_time = difftime(stop_time, start_time);
    pthread_mutex_unlock(&waiting_room);

    consultatie(doctor_id, patient, total_time, late_time);

    sem_post(&semaphore);
}
```

* Consulation  
It will calculate the actual arrival time, completion time, turnaround time and it will print all the values in a table.
```C
void consultatie(int doctor_index, int patient, time_t waited_time, time_t late_time)
{
    pthread_mutex_lock(&doctors[doctor_index].mutex);
    patients[patient].waiting_time = waited_time;
    patients[patient].completion_time = patients[patient].arrival_time +
                                        patients[patient].burst_time +
                                        patients[patient].waiting_time +
                                        late_time;
    patients[patient].turnaround_time = patients[patient].burst_time +
                                        patients[patient].waiting_time +
                                        late_time;
    patients[patient].arrival_time = patients[patient].arrival_time + waited_time;
    if(late_time > 0)
    {
        patients[patient].arrival_time += late_time;
    }

    print_values(doctor_index,
                patient,
                patients[patient].arrival_time,
                patients[patient].burst_time,
                patients[patient].completion_time,
                patients[patient].turnaround_time,
                waited_time,
                late_time);

    pthread_mutex_unlock(&doctors[doctor_index].mutex);

    sleep(patients[patient].burst_time);

    pthread_mutex_lock(&doctors[doctor_index].mutex);
    enqueue(&available_doctors, doctor_index);
    pthread_mutex_unlock(&doctors[doctor_index].mutex);
}
```

* Final result 
![Run example](https://github.com/anacomo/MedicalOfficeSimulator/blob/main/example.jpg)
