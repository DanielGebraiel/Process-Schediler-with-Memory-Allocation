// Implementation from geeks for geeks :)
// URL: https://www.geeksforgeeks.org/priority-queue-using-linked-list/
// Approved by TA Muhammed Hesham

// C code to implement Priority Queue
// using Linked List
#include <stdio.h>
#include <stdlib.h>
#include "process.h"

// Node
typedef struct node
{
	process proc;

	// Lower values indicate higher priority
	int priority;

	struct node *next;

} Node;

void printQueue(Node **head)
{
	Node *temp = *head;
	while (temp != NULL)
	{
		printf("%d ", temp->proc.id);
		temp = temp->next;
	}
	printf("\n");
}


// Function to Create A New Node
Node *newNode(int id, int arrivalTime, int runningTime, int remainingTime, int PID, int waitTime, int finishTime, int startTime, int lastTimeActive,int priority, int memorySize, int startAddress, int endAddress)
{
	Node *temp = (Node *)malloc(sizeof(Node));

	temp->proc.id = id;
	temp->proc.arrivalTime = arrivalTime;
	temp->proc.runningTime = runningTime;
	temp->proc.priority = priority;
	temp->priority = priority;
	temp->proc.remainingTime = remainingTime;
	temp->proc.waitTime = waitTime;
	temp->proc.finishTime = finishTime;
	temp->proc.startTime = startTime;
	temp->proc.lastTimeActive = lastTimeActive;
	temp->proc.PID = PID;
	temp->proc.memorySize = memorySize;
	temp->proc.startAddress = startAddress;
	temp->proc.endAddress = endAddress;
	temp->next = NULL;
	return temp;
}

// Return the value at head
process peek(Node **head)
{
	return (*head)->proc;
}

// Removes the element with the
// highest priority from the list
void pop(Node **head)
{
	Node *temp = *head;
	(*head) = (*head)->next;
	free(temp);
	printQueue(head);
}

// Function to push according to priority
void push(Node **head, int id, int arrivalTime, int runningTime, int remainingTime, int PID, int waitTime, int finishTime, int startTime, int lastTimeActive,int priority, int memorySize, int startAddress, int endAddress)
{
	Node *start = (*head);

	// Create new Node
	Node *temp = newNode(id, arrivalTime, runningTime, remainingTime, PID, waitTime, finishTime, startTime, lastTimeActive,priority,memorySize,startAddress,endAddress);

	// Special Case: The head of list has lesser
	// priority than new node. So insert new
	// node before head node and change head node.
	if ((*head)->priority > priority)
	{

		// Insert New Node before head
		temp->next = *head;
		(*head) = temp;
	}
	else
	{

		// Traverse the list and find a
		// position to insert new node
		while (start->next != NULL && start->next->priority <= priority)
		{
			start = start->next;
		}

		// Either at the ends of the list
		// or at required position
		temp->next = start->next;
		start->next = temp;
	}
	printQueue(head);
}

// Function to check is list is empty
int isEmpty(Node **head)
{
	return (*head) == NULL;
}

