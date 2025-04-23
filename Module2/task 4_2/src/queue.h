#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    int priority;
    int data;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} PriorityQueue;

// Основные функции работы с очередью
void enqueue(PriorityQueue *q, int priority, int data);
Node* dequeue(PriorityQueue *q);
Node* extract_by_priority(PriorityQueue *q, int priority);
Node* extract_by_min_priority(PriorityQueue *q, int min_priority);
void print_queue(PriorityQueue *q);

#endif
