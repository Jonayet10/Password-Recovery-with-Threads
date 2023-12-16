#include "thread_pool.h"

#include <pthread.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"

typedef struct thread_pool {
    queue_t *queue;            // Shared task queue for the thread pool
    pthread_t *workers;        // Array to hold worker TIDs
    size_t num_worker_threads; // Number of worker threads in the pool
} thread_pool_t;

typedef struct task {
    work_function_t func; // Function to be executed by a worker
    void *arg;            // Argument to be passed to the function
} task_t;

// Entry point for worker threads to execute tasks
void *execute_tasks(void *task_queue) {
    queue_t *queue = (queue_t *) task_queue;

    while (true) {
        // Dequeue a task from the shared task queue
        void *item = queue_dequeue(queue);

        // Check if it's a NULL task (exit signal)
        if (item == NULL) {
            return NULL; // Exit the worker thread
        }

        // Cast the queue item as a task struct
        task_t *work = (task_t *) item;

        // Apply the assigned function to its argument
        (work->func)(work->arg);

        // Free the memory specifically allocated for the task item
        free(item);
    }

    return NULL;
}

// Initializes a thread pool with the specified number of worker threads
thread_pool_t *thread_pool_init(size_t num_worker_threads) {
    pthread_t *threads = (pthread_t *) calloc(num_worker_threads, sizeof(pthread_t));
    assert(threads != NULL);

    thread_pool_t *thread_pool = (thread_pool_t *) malloc(sizeof(thread_pool_t));
    assert(thread_pool != NULL);

    thread_pool->queue = queue_init(); // Initialize the shared task queue

    thread_pool->workers = threads; // Set pointer to array of TIDs
    thread_pool->num_worker_threads =
        num_worker_threads; // Set the number of worker threads

    // Create worker threads and assign the 'execute_tasks' function
    for (size_t i = 0; i < num_worker_threads; i++) {
        pthread_create(&threads[i], NULL, execute_tasks, thread_pool->queue);
    }
    return thread_pool;
}

// Add work to the thread pool
void thread_pool_add_work(thread_pool_t *pool, work_function_t function, void *aux) {
    task_t *work = (task_t *) malloc(sizeof(task_t));
    if (work == NULL) {
        return; // Allocation failure, couldn't add the work
    }

    work->func = function; // Assign the function to be executed
    work->arg = aux;       // Assign the argument to be passed to the function

    // Enqueue the work item in the shared task queue
    queue_enqueue(pool->queue, (void *) work);
}

// Finish and clean up the thread pool
void thread_pool_finish(thread_pool_t *pool) {
    // Signal all worker threads to exit by enqueuing NULL tasks
    for (size_t i = 0; i < pool->num_worker_threads; i++) {
        queue_enqueue(pool->queue, NULL);
    }

    // Wait for all worker threads to complete
    for (size_t j = 0; j < pool->num_worker_threads; j++) {
        pthread_join(pool->workers[j], NULL);
    }

    // Free resources associated with the thread pool
    queue_free(pool->queue);
    free(pool->workers);
    free(pool);
}
