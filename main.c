#include "queue.c"

void consultatie(int doctor_index, int patient, time_t waited_time, time_t late_time)
{
    pthread_mutex_lock(&doctors[doctor_index].mutex);
    // trebuie sa il pun sa astepte
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

void *send_to_consultation(void *arg)
{
    int patient = *((int *)arg);
    waiting(patient, 0);
    while (isAnyDoctorAvailable(&patients[patient].arrival_times_queue))
    {
        //printf("s-a reluat thread-ul %d\n", patient);
        pthread_mutex_lock(&resend);
        patients[patient].arrival_time = dequeue(&patients[patient].arrival_times_queue);
        patients[patient].burst_time = dequeue(&patients[patient].burst_times_queue);
        time_t late_time = difftime(patients[patient].completion_time, patients[patient].arrival_time);
        //printf("last completion time: %ld, next arrival time: %ld, late time : %ld\n", patients[patient].completion_time, patients[patient].arrival_time, late_time);
        if (patients[patient].completion_time > patients[patient].arrival_time)
        {
            //printf("arrived late\n");
            pthread_mutex_unlock(&resend);
        }
        else
        {
            //printf("va ajunge mai tarziu cu %ld\n", patients[patient].arrival_time - patients[patient].completion_time);
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

int main(int argc, char *argv[])
{
    //file = fopen("generate.txt", "r");
    file = fopen("exemplu.txt", "r");
    initialize();
    print_header();

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

    destroy();
    fclose(file);

    return 0;
}