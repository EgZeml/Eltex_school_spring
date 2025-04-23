#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// добавить в очередь
void enqueue(PriorityQueue *q, int priority, int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }
    new_node->priority = priority;
    new_node->data = data;
    new_node->next = NULL;

    if (q->head == NULL) {
        q->head = q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
}

// Достать из очереди элемент с заданным элементом
Node* extract_by_priority(PriorityQueue *q, int priority) {
    if (q->head == NULL) {
        return NULL;
    }   

    Node* prev = NULL;
    Node* current = q->head;
    while (current != NULL) {
        if (current->priority == priority) {
            if (current == q->head) {
                q->head = q->head->next;
                if (q->head == NULL) {
                    q->tail = NULL;
                } 
            } else if (current == q->tail) {
                prev->next = NULL;
                q->tail = prev;
            } else {
                prev->next = current->next;
            }
            return current;
        }

        prev = current;
        current = current->next;
    }
    return NULL;
}

// Извлечение элемента с наивысшим приоритетом (наименьшее значение)
Node* dequeue(PriorityQueue *q) {
    if (q->head == NULL) {
        return NULL;
    }
    int best_priority = q->head->priority;
    Node* current_node = q->head->next;
    while (current_node != NULL) {
        if (current_node->priority < best_priority) {
            best_priority = current_node->priority;
        }
        current_node = current_node->next;
    }
    return extract_by_priority(q, best_priority);
}

// Извлечение элемента с приоритетом не ниже заданного
Node* extract_by_min_priority(PriorityQueue *q, int min_priority) {
    int best_priority = 256;
    Node* current_node = q->head;
    int found = 0;
    while (current_node != NULL) {
        if (current_node->priority >= min_priority && current_node->priority < best_priority) {
            best_priority = current_node->priority;
            found = 1;
        }
        current_node = current_node->next;
    }
    if (found) {
        return extract_by_priority(q, best_priority);
    }
    return NULL;
}

// Вывод очереди
void print_queue(PriorityQueue *q) {
    Node* current = q->head;
    printf("Очередь: ");
    if (current == NULL) {
        printf("Пустая");
    }
    while (current != NULL) {
        printf("[Приоритет: %3d, Данные: %d] -> ", current->priority, current->data);
        current = current->next;
    }
    printf("NULL\n");
}