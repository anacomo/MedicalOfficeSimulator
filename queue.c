#include "main.h"

void init_queue(queue *q)
{
    q->head = NULL;
    q->tail = NULL;
}

bool enqueue(queue *q, int doctor_id)
{
    node *newnode = malloc(sizeof(node));
    if (newnode == NULL)
    {
        return false;
    }

    newnode->doctor_id = doctor_id;
    newnode->next = NULL;
    if (q->tail != NULL)
    {
        q->tail->next = newnode;
    }
    q->tail = newnode;
    if (q->head == NULL)
    {
        q->head = newnode;
    }
    return true;
}

int dequeue(queue *q)
{
    if (q->head == NULL)
    {
        return 0;
    }
    node *tmp = q->head;
    int result = tmp->doctor_id;
    q->head = q->head->next;
    if (q->head == NULL)
    {
        q->tail = NULL;
    }
    free(tmp);
    return result;
}

bool isAnyDoctorAvailable(queue *q)
{
    if (q->head == NULL)
        return false;
    return true;
}

queue available_doctors;

void initialize()
{
    /* patient threads */
    for (int i = 0; i < nr_patients; i++)
    {
        init_queue(&patients[i].arrival_times_queue);
        init_queue(&patients[i].burst_times_queue);

        fscanf(file, "%d ", &patients[i].nr_consultations);
        for (int j = 1; j <= patients[i].nr_consultations; j++)
        {
            // citeste valoare
            time_t x, y;
            fscanf(file, "%ld %ld", &x, &y);

            enqueue(&patients[i].arrival_times_queue, x);
            enqueue(&patients[i].burst_times_queue, y);
            patients[i].arrival_time = x;
            patients[i].completion_time = y;
        }
        patients[i].arrival_time = dequeue(&patients[i].arrival_times_queue);
        patients[i].burst_time = dequeue(&patients[i].burst_times_queue);
    }

    // initialize doctor structure
    init_queue(&available_doctors);

    for (int i = 0; i < doctor_numbers; ++i)
    {
        doctors[i].available = 1;
        doctors[i].id = i;
        enqueue(&available_doctors, i);
        if (pthread_mutex_init(&doctors[i].mutex, NULL))
        {
            printf("Error trying to initialize mutex!");
        }
    }

    if (pthread_mutex_init(&waiting_room, NULL))
    {
        printf("Error trying to initialize mutex!");
    }

    if (pthread_mutex_init(&resend, NULL))
    {
        printf("Error trying to initialize mutex!");
    }

    // initialize semaphore
    if (sem_init(&semaphore, 0, doctor_numbers))
    {
        printf("Error trying to initialize semaphore!");
    }
}

void destroy()
{
    // join the threads
    for (int i = 0; i < nr_patients; i++)
    {
        if (pthread_join(patients[i].my_thread, NULL))
        {
            printf("Error trying to join threads!");
        }
    }

    // destroy mutexes
    for (int i = 0; i < doctor_numbers; i++)
    {
        if (pthread_mutex_destroy(&doctors[i].mutex))
        {
            printf("Error trying to destroy mutex!");
        }
    }

    if (pthread_mutex_destroy(&waiting_room))
    {
        printf("Error trying to destroy mutex!");
    }

    if (pthread_mutex_destroy(&resend))
    {
        printf("Error trying to destroy resend mutex!");
    }

    while (isAnyDoctorAvailable(&available_doctors))
    {
        dequeue(&available_doctors);
    }

    sem_destroy(&semaphore);
}

void print_header()
{
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");
    printf("|  Doc ID  |  Patient ID  |  Arrival time  |  Burst time  |  Completion time  |  Turnaround time  |  Waited time  |  Late time  |\n");
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");
}

void print_values(int doc_id, int pid, time_t a, time_t b, time_t c, time_t t, time_t w, time_t l)
{
    if (doc_id < 10)
    {
        printf("|     %d    ", doc_id);
    }
    else if (doc_id < 100)
    {
        printf("|    %d    ", doc_id);
    }
    else
    {
        printf("|   %d    ", doc_id);
    }

    if (pid < 10)
    {
        printf("|       %d      ", pid);
    }

    if (a < 10)
    {
        printf("|        %ld       ", a);
    }
    else if (a < 100)
    {
        printf("|       %ld       ", a);
    }
    else
    {
        printf("|      %ld       ", a);
    }

    if (b < 10)
    {
        printf("|       %ld      ", b);
    }
    else if (b < 100)
    {
        printf("|      %ld      ", b);
    }
    else
    {
        printf("|     %ld      ", b);
    }

    if (c < 10)
    {
        printf("|          %ld        ", c);
    }
    else if (c < 100)
    {
        printf("|         %ld        ", c);
    }
    else
    {
        printf("|        %ld        ", c);
    }

    if (t < 10)
    {
        printf("|          %ld        ", t);
    }
    else if (t < 100)
    {
        printf("|         %ld        ", t);
    }
    else
    {
        printf("|        %ld        ", t);
    }

    if (w < 10)
    {
        printf("|       %ld       ", w);
    }
    else if (w < 100)
    {
        printf("|      %ld       ", w);
    }
    else
    {
        printf("|      %ld       ", w);
    }

    if (l < 10)
    {
        printf("|      %ld      |\n", l);
    }
    else if (l < 100)
    {
        printf("|     %ld      |\n", l);
    }
    else
    {
        printf("|    %ld      |\n", l);
    }
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");
}