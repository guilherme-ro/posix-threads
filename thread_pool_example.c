#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_THREADS 4
#define NUM_TASKS   10

// Estrutura para uma tarefa
typedef struct {
    int id;
    int duration; // tempo simulado em segundos
} Task;

// Estrutura da fila de tarefas
typedef struct {
    Task *tasks;
    int front, rear, size, capacity;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    bool done; // indica que não há mais tarefas a adicionar
} TaskQueue;

// Inicializa a fila
void task_queue_init(TaskQueue *queue, int capacity) {
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    queue->done = false;
    queue->tasks = malloc(capacity * sizeof(Task));
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
}

// Destroi a fila
void task_queue_destroy(TaskQueue *queue) {
    free(queue->tasks);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
}

// Adiciona tarefa na fila (bloqueia se cheia)
void task_queue_push(TaskQueue *queue, Task task) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->tasks[queue->rear] = task;
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
}

// Remove tarefa da fila (bloqueia se vazia, respeita 'done')
Task task_queue_pop(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0 && !queue->done) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    if (queue->size == 0 && queue->done) {
        pthread_mutex_unlock(&queue->mutex);
        Task empty = {-1, 0};
        return empty; // sinaliza fim
    }
    Task task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

// Função da thread trabalhadora
void* worker_thread(void* arg) {
    TaskQueue *queue = (TaskQueue*)arg;
    while (true) {
        Task task = task_queue_pop(queue);
        if (task.id == -1) break; // sinal de término

        printf("[Thread %ld] Processando tarefa %d (duração: %ds)\n",
               pthread_self(), task.id, task.duration);
        sleep(task.duration);
        printf("[Thread %ld] Tarefa %d concluída!\n", pthread_self(), task.id);
    }
    printf("[Thread %ld] Encerrando.\n", pthread_self());
    return NULL;
}

int main() {
    TaskQueue queue;
    task_queue_init(&queue, 5); // fila com capacidade 5

    pthread_t threads[NUM_THREADS];
    long i;

    // Cria threads trabalhadoras
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, &queue) != 0) {
            perror("Falha ao criar thread");
            exit(1);
        }
    }

    // Produtor: adiciona tarefas
    for (i = 0; i < NUM_TASKS; i++) {
        Task t = { (int)i, 1 + (rand() % 3) }; // duração entre 1 e 3s
        printf("[Main] Adicionando tarefa %d\n", t.id);
        task_queue_push(&queue, t);
        usleep(300000); // simula produção gradual
    }

    // Sinaliza que não há mais tarefas
    pthread_mutex_lock(&queue.mutex);
    queue.done = true;
    pthread_cond_broadcast(&queue.not_empty); // acorda todas as threads
    pthread_mutex_unlock(&queue.mutex);

    // Espera todas as threads terminarem
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    task_queue_destroy(&queue);
    printf("Todas as tarefas foram processadas. Programa finalizado.\n");
    return 0;
}
