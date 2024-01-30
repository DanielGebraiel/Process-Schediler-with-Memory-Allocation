#include <stdio.h>
#include <stdlib.h>
#include "priorityQueue.h"

// Structure for Queue Node
typedef struct QNode
{
    process proc;
    struct QNode *next;
} QNode;

QNode *newQNode(int id, int arrivalTime, int runningTime, int remainingTime, int PID, int waitTime, int finishTime, int startTime, int lastTimeActive,int priority,int memSize,int startAddress,int endAddress)
{
	QNode *temp = (QNode *)malloc(sizeof(QNode));

	temp->proc.id = id;
	temp->proc.arrivalTime = arrivalTime;
	temp->proc.runningTime = runningTime;
	temp->proc.priority = priority;
	temp->proc.remainingTime = remainingTime;
	temp->proc.waitTime = waitTime;
	temp->proc.finishTime = finishTime;
	temp->proc.startTime = startTime;
    temp->proc.lastTimeActive = lastTimeActive;
	temp->proc.PID = PID;
    temp->proc.memorySize = memSize;
    temp->proc.startAddress = startAddress;
    temp->proc.endAddress = endAddress;
    temp->next = NULL;
	return temp;
}

// Structure for the circular queue
typedef struct
{
    QNode *front, *rear;
} CircularQueue;

// Function to initialize the circular queue
void initQueue(CircularQueue *q)
{
    q->front = q->rear = NULL;
}

// Function to check if the queue is empty
int isEmptyCirc(CircularQueue *q)
{
    return q->front == NULL;
}

// Function to add a process to the queue
void enqueue(CircularQueue *q, QNode *newProcess)
{
    if (isEmptyCirc(q))
    {
        q->front = q->rear = newProcess;
        newProcess->next = newProcess; // Circular queue with one element points to itself
    }
    else
    {
        newProcess->next = q->front;
        q->rear->next = newProcess;
        q->rear = newProcess;
    }
}

// Function to remove a process from the queue
QNode *dequeue(CircularQueue *q)
{
    QNode *removedProcess = NULL;
    if (isEmptyCirc(q))
    {
        printf("Queue is empty, cannot dequeue process.\n");
    }
    else if (q->front == q->rear)
    {
        removedProcess = q->front;
        q->front = q->rear = NULL;
    }
    else
    {
        removedProcess = q->front;
        q->front = q->front->next;
        q->rear->next = q->front;
    }
    return removedProcess;
}

// Function to display the queue
void display(CircularQueue *q)
{
    if (isEmptyCirc(q))
    {
        printf("Queue is empty.\n");
        return;
    }

    QNode *current = q->front;
    printf("Queue elements: ");
    do
    {
        printf("ID: %d ", current->proc.id);
        current = current->next;
    } while (current != q->front);
    printf("\n");
}