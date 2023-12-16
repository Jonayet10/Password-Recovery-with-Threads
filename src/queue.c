#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

// Defines a structure for each item in the queue
typedef struct item {
    void *val;         // Pointer to the value stored in this item
    struct item *next; // Pointer to the next item in the queue
} item_t;

// Defines a structure for the queue
typedef struct queue {
    item_t *head;             // Pointer to the first item in the queue
    item_t *tail;             // Pointer to the last item in the queue
    pthread_mutex_t lock;     // Mutex for thread safety (preventing race conditions)
    pthread_cond_t condition; // Condition variable for blocking dequeue
} queue_t;

// Initialize a new queue
queue_t *queue_init(void) {
    queue_t *queue = (queue_t *) malloc(sizeof(queue_t));

    if (queue == NULL) {
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;

    // Initialize the mutex and condition variable
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->condition, NULL);
    return queue;
}

// Enqueue an item into the queue
void queue_enqueue(queue_t *queue, void *val) {
    pthread_mutex_lock(&queue->lock);

    item_t *to_append = (item_t *) malloc(sizeof(item_t));
    to_append->val = val;
    to_append->next = NULL;

    if ((queue->head == NULL) && (queue->tail == NULL)) {
        // If the queue is empty, set both head and tail to the new item
        queue->head = to_append;
        queue->tail = to_append;
    }
    else {
        // Otherwise, append the new item to the tail
        queue->tail->next = to_append; // Set next of current tail to to_append
        queue->tail = to_append;       // Set the to_append as the tail
    }

    // Signal waiting threads that an item has been enqueued
    pthread_cond_signal(&queue->condition);
    pthread_mutex_unlock(&queue->lock);
}

// Dequeue an item from the queue (blocking if the queue is empty)
void *queue_dequeue(queue_t *queue) {
    if (queue == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&queue->lock);

    // Wait until the queue is not empty
    while ((queue->head == NULL) && (queue->tail == NULL)) {
        pthread_cond_wait(&queue->condition, &queue->lock);
    }

    item_t *old_head = queue->head;
    void *val = old_head->val;

    if (queue->head == queue->tail) {
        // If there's only one item in the queue, reset head and tail
        free(queue->head);
        queue->head = NULL;
        queue->tail = NULL;
        pthread_mutex_unlock(&queue->lock);
        return val;
    }

    // Move the head pointer to the next item and free the old head to dequeue the head
    queue->head = queue->head->next;
    free(old_head);
    pthread_mutex_unlock(&queue->lock);
    return val;
}

// Free the memory allocated for the queue and its items
void queue_free(queue_t *queue) {
    if (queue == NULL) {
        return;
    }

    // Free all queue items
    item_t *current = queue->head;
    while (current != NULL) {
        item_t *next = current->next;
        free(current);
        current = next;
    }

    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->condition);

    // Free the memory allocated for the queue
    free(queue);
}
