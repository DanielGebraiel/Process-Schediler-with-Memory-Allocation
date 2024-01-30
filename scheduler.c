#include "headers.h"
#include <math.h>

int main(int argc, char *argv[])
{
    // Connect the scheduler to the clock
    initClk();

    // Getting inputs from the process generator
    int totalNumOfProcesses = atoi(argv[1]);
    int chosenAlgo = atoi(argv[2]);
    int quantum = atoi(argv[3]);

    printf("Number of procs sent: %d\n", totalNumOfProcesses);

    // Establishing the message queue to recieve from the process generator
    int recVal;
    int processMsgqID = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (processMsgqID == -1)
    {
        perror("Error in creating process message queue! :(");
        exit(-1);
    }

    ////////////////////////////////// Creating Output files //////////////////////////////////
    FILE *schedulerLog = fopen("scheduler.log", "w");

    // Check if the file was opened successfully
    if (schedulerLog == NULL)
    {
        perror("Error opening the file. :( \n");
        exit(-1);
    }

    fprintf(schedulerLog, "#At time x process y state arr w total z remain y wait k\n");

    FILE *schedulerPerf = fopen("scheduler.perf", "w");

    // Check if the file was opened successfully
    if (schedulerPerf == NULL)
    {
        perror("Error opening the file. :( \n");
        exit(-1);
    }

    FILE *memoryLog = fopen("memory.log", "w");

    // Check if the file was opened successfully
    if (memoryLog == NULL)
    {
        perror("Error opening the file. :( \n");
        exit(-1);
    }
    fprintf(memoryLog, "#At time x allocated y bytes for process z from i to j\n");
    ///////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////  General Declarations //////////////////////////////////

    // Buffer to recieve the processes
    processBuff processJustRecieved;
    // Number of processes recieved so far
    int numOfRecievedProcesses = 0;
    // Indicates whether the scheduler is idle
    bool isIdle = true;
    // PID of current running process
    int currentPID = 0;
    // Total running time to be used in CPU utilization
    int totalRunningTime = 0;
    // To be used in .perf file
    int totalWaitingTime = 0;
    float totalWeightedTurnAroundTime = 0;
    float *weightedTurnAroundTimes = (float *)malloc(totalNumOfProcesses * sizeof(float));
    int numOfProcessesFinished;
    // declare binary tree root as memory initialization
    treeNode *root = newTreeNode(1024, 0, 1023);

    ///////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////// HPF ///////////////////////////////////////////
    if (chosenAlgo == HPF)
    {
        //////////////////////////////////// Declarations /////////////////////////////////////

        // Initialize priority Queue
        Node *readyQueueHead = NULL;

        process currentRunningProcess;
        int currentProcessFinishTime = -1;
        int allocatedMemory = 0;
        Node *waitQueueHead = NULL;
        process currentWaitingProcess;

        ///////////////////////////////////////////////////////////////////////////////////////

        // Keep working until recieved all proceses, no elements are left in the priority queue and the scheduler has finished all his processes
        while (numOfRecievedProcesses < totalNumOfProcesses || !isIdle || readyQueueHead != NULL)
        {
            // Another while loop to recieve all processes that arrive at the same time before continuing
            while (numOfRecievedProcesses < totalNumOfProcesses)
            {
                // Recieve the process
                recVal = msgrcv(processMsgqID, &processJustRecieved, sizeof(processJustRecieved), 0, IPC_NOWAIT);

                // Checks if a process was sent
                if (recVal != -1)
                {
                    printf("Recieved!\n");
                    // Increasing total running Time
                    totalRunningTime += processJustRecieved.procsAttributes[2];

                    treeNode *found = allocateProcess(root, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[4]);
                    // process can fit in memory, so it is placed in readQueue
                    if (found != NULL)
                    {
                        if (readyQueueHead == NULL)
                        {
                            readyQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], found->start, found->end);
                        }
                        else
                        {
                            push(&readyQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], found->start, found->end);
                        }
                        fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), processJustRecieved.procsAttributes[4], processJustRecieved.procsAttributes[0], found->start, found->end);
                        printTree(root);
                    }
                    else
                    {
                        // Put in waitQueue (ordered by arrival time)
                        if (waitQueueHead == NULL)
                        {
                            waitQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                        else
                        {
                            push(&waitQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                    }

                    numOfRecievedProcesses++;
                }
                // No process sent so break the recieving loop
                else
                {
                    break;
                }
            }

            // Scheduler is idle, gets new process if exists
            if (readyQueueHead != NULL && isIdle)
            {
                // Put new process in currentRunningProcess
                currentRunningProcess = peek(&readyQueueHead);
                pop(&readyQueueHead);

                // Set its start time abd wait time
                currentRunningProcess.startTime = getClk();
                currentRunningProcess.waitTime = currentRunningProcess.startTime - currentRunningProcess.arrivalTime;

                // Fork the new process and save its PID
                currentPID = fork();
                // allocate the forked proccess in memory
                currentRunningProcess.PID = currentPID;

                if (currentPID == 0)
                {
                    // Sending the start time and run time to the process
                    char startTimeStr[20];
                    char remainingTimeStr[20];

                    sprintf(startTimeStr, "%d", currentRunningProcess.startTime);
                    sprintf(remainingTimeStr, "%d", currentRunningProcess.runningTime);

                    printf("Scheduler: Process ID %d has started running at %d\n", currentRunningProcess.id, getClk());

                    // Use these strings as arguments in execl
                    if (execl("./process.out", "process.out", startTimeStr, remainingTimeStr, NULL) == -1)
                    {
                        perror("Can't open process.out! :(");
                        exit(-1);
                    }
                }

                printTree(root);
                fprintf(schedulerLog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                // To indicate that the scheduler is currently running a process
                isIdle = false;
                // To know when the process finishes
                currentProcessFinishTime = getClk() + currentRunningProcess.runningTime;
                // Setting the process' finish time
                currentRunningProcess.finishTime = currentProcessFinishTime;
            }

            // Scheduler has just finished the process and becomes idle
            if (getClk() == currentProcessFinishTime)
            {
                isIdle = true;
                currentRunningProcess.remainingTime = 0;

                printf("Scheduler: Process ID %d finished at time %d\n", currentRunningProcess.id, currentProcessFinishTime);
                // deallocate the process from memory
                deallocateProcess(root, currentRunningProcess.id);
                fprintf(memoryLog, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), currentRunningProcess.memorySize, currentRunningProcess.id, currentRunningProcess.startAddress, currentRunningProcess.endAddress);
                // Waits for forked process to finish execution
                printTree(root);
                waitpid(currentRunningProcess.PID, NULL, 0);
                while (waitQueueHead != NULL)
                {
                    currentWaitingProcess = peek(&waitQueueHead);
                    pop(&waitQueueHead);
                    treeNode *found = allocateProcess(root, currentWaitingProcess.id, currentWaitingProcess.memorySize); 
                    if (found != NULL)                                                                                   
                    {
                        if (readyQueueHead == NULL)
                        {
                            readyQueueHead = newNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.priority, currentWaitingProcess.memorySize, found->start, found->end);
                        }
                        else
                        {
                            push(&readyQueueHead, currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.priority, currentWaitingProcess.memorySize, found->start, found->end);
                        }
                        currentWaitingProcess.startAddress = found->start;
                        currentWaitingProcess.endAddress = found->end;
                        fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), currentWaitingProcess.memorySize, currentWaitingProcess.id, currentWaitingProcess.startAddress, currentWaitingProcess.endAddress);
                    }
                    else
                    {
                        if (waitQueueHead == NULL)
                        {
                            waitQueueHead = newNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.priority, currentWaitingProcess.memorySize, -1, -1);
                        }
                        else
                        {
                            push(&waitQueueHead, currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.priority, currentWaitingProcess.memorySize, -1, -1);
                        }
                        break;
                    }
                }
                // Calculating statistics for output file

                float weightedTurnAroundTime = (float)(currentRunningProcess.finishTime - currentRunningProcess.arrivalTime) / currentRunningProcess.runningTime;
                totalWeightedTurnAroundTime += weightedTurnAroundTime;
                totalWaitingTime += currentRunningProcess.waitTime;
                weightedTurnAroundTimes[numOfProcessesFinished] = weightedTurnAroundTime;
                numOfProcessesFinished++;

                fprintf(schedulerLog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime, currentRunningProcess.finishTime - currentRunningProcess.arrivalTime, weightedTurnAroundTime);
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////// SRTN //////////////////////////////////////////
    else if (chosenAlgo == SRTN)
    {
        //////////////////////////////////// Declarations /////////////////////////////////////

        Node *readyQueueHead = NULL;
        process currentRunningProcess;
        int currentRunningTime;
        int processResumingTime;
        int allocatedMemory = 0;

        // declaring wait queue for processes that don't fit in the memory
        Node *waitQueueHead = NULL;
        process currentWaitingProcess;

        // Keep working until recieved all proceses, no elements are left in the priority queue and the scheduler has finished all his processes
        while (numOfRecievedProcesses < totalNumOfProcesses || !isIdle || readyQueueHead != NULL)
        {
            // Another while loop to recieve all processes that arrive at the same time before continuing
            while (numOfRecievedProcesses < totalNumOfProcesses)
            {
                // Recieve the process
                recVal = msgrcv(processMsgqID, &processJustRecieved, sizeof(processJustRecieved), 0, IPC_NOWAIT);

                // Checks if a process was sent
                if (recVal != -1)
                {
                    printf("Recieved!\n");
                    // Increasing total running Time
                    totalRunningTime += processJustRecieved.procsAttributes[2];

                    treeNode *found = allocateProcess(root, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[4]);
                    // process can fit in memory, so it is placed in readQueue
                    if (found != NULL)
                    {
                        if (readyQueueHead == NULL)
                        {
                            readyQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[4], found->start, found->end);
                        }
                        else
                        {
                            push(&readyQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[4], found->start, found->end);
                        }
                        fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), processJustRecieved.procsAttributes[4], processJustRecieved.procsAttributes[0], found->start, found->end);
                        printTree(root);
                    }
                    else
                    {
                        // Put in waitQueue (ordered by arrival time)
                        if (waitQueueHead == NULL)
                        {
                            waitQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                        else
                        {
                            push(&waitQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                    }

                    numOfRecievedProcesses++;
                }
                // No process sent so break the recieving loop
                else
                {
                    break;
                }
            }

            // The scheduler is currently working on a process
            if (!isIdle)
            {
                // We get the time that the process has ran so far
                currentRunningTime = getClk() - processResumingTime;

                // If the remaining time equals the running time then the process has finished
                if (currentRunningProcess.remainingTime == currentRunningTime)
                {
                    printf("Scheduler: Process ID %d has finished at time %d byeeee\n", currentRunningProcess.id, getClk());

                    // Waits for forked process to finish execution
                    // waitpid(currentRunningProcess.PID, NULL, 0);

                    // Calculating statistics for output file
                    float weightedTurnAroundTime = (float)(getClk() - currentRunningProcess.arrivalTime) / currentRunningProcess.runningTime;
                    totalWeightedTurnAroundTime += weightedTurnAroundTime;
                    totalWaitingTime += currentRunningProcess.waitTime;
                    weightedTurnAroundTimes[numOfProcessesFinished] = weightedTurnAroundTime;
                    numOfProcessesFinished++;
                    deallocateProcess(root, currentRunningProcess.id);
                    fprintf(schedulerLog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime - currentRunningTime, currentRunningProcess.waitTime, getClk() - currentRunningProcess.arrivalTime, weightedTurnAroundTime);
                    fprintf(memoryLog, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), currentRunningProcess.memorySize, currentRunningProcess.id, currentRunningProcess.startAddress, currentRunningProcess.endAddress);
                    printf("current process id %d \n", currentRunningProcess.id);

                    printTree(root);
                    ///////////////////////////////////////////////////
                    // Check if there are processes in waitQueue that can be placed in readyQueue now (as a process was terminated)

                    while (waitQueueHead != NULL)
                    {
                        currentWaitingProcess = peek(&waitQueueHead);
                        pop(&waitQueueHead);
                        treeNode *found = allocateProcess(root, currentWaitingProcess.id, currentWaitingProcess.memorySize); 
                        if (found != NULL)                                                                                   
                        {
                            if (readyQueueHead == NULL)
                            {
                                readyQueueHead = newNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.remainingTime, currentWaitingProcess.memorySize, found->start, found->end);
                            }
                            else
                            {
                                push(&readyQueueHead, currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.remainingTime, currentWaitingProcess.memorySize, found->start, found->end);
                            }
                            currentWaitingProcess.startAddress = found->start;
                            currentWaitingProcess.endAddress = found->end;
                            fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), currentWaitingProcess.memorySize, currentWaitingProcess.id, currentWaitingProcess.startAddress, currentWaitingProcess.endAddress);
                        }
                        else
                        {
                            if (waitQueueHead == NULL)
                            {
                                waitQueueHead = newNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.remainingTime, currentWaitingProcess.memorySize, -1, -1);
                            }
                            else
                            {
                                push(&waitQueueHead, currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.remainingTime, currentWaitingProcess.memorySize, -1, -1);
                            }
                            break;
                        }
                    }
                    /////////////////////////////////////////////////////
                    isIdle = true;
                }

                // Checks if there are still processes in the ready queue
                else if (readyQueueHead != NULL)
                {
                    // Checks if the remaining time of the running process is greater than the smallest running time in ready queue
                    if (currentRunningProcess.remainingTime - currentRunningTime > peek(&readyQueueHead).remainingTime)
                    {
                        // We stop the current process
                        kill(currentRunningProcess.PID, SIGSTOP);

                        // If true then we pre-empt the current process
                        printf("Premption!\n");
                        printf(" %d vs %d\n", currentRunningProcess.remainingTime - currentRunningTime, peek(&readyQueueHead).remainingTime);

                        // We update the remaining time
                        currentRunningProcess.remainingTime = currentRunningProcess.remainingTime - currentRunningTime;
                        currentRunningProcess.lastTimeActive = getClk();

                        // We push the process back the ready queue
                        printf("Scheduler: Process ID %d has stopped at %d and has remaining time %d\n", currentRunningProcess.id, getClk(), currentRunningProcess.remainingTime);
                        fprintf(schedulerLog, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                        push(&readyQueueHead, currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.PID, currentRunningProcess.waitTime, currentRunningProcess.finishTime, currentRunningProcess.startTime, currentRunningProcess.lastTimeActive, currentRunningProcess.remainingTime, currentRunningProcess.memorySize, currentRunningProcess.startAddress, currentRunningProcess.endAddress);

                        // We put the new process in the currentRunningProcess
                        currentRunningProcess = peek(&readyQueueHead);
                        pop(&readyQueueHead);

                        // If the process hasn't been forked it we fork it
                        if (currentRunningProcess.PID == -1)
                        {
                            // We update the start time, wait time and the PID of the process
                            processResumingTime = getClk();
                            currentRunningProcess.waitTime = getClk() - currentRunningProcess.arrivalTime;
                            fprintf(schedulerLog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                            currentPID = fork();
                            currentRunningProcess.startTime = getClk();
                            currentRunningProcess.PID = currentPID;
                            if (currentPID == 0)
                            {
                                char startTimeStr[20];
                                char remainingTimeStr[20];

                                sprintf(startTimeStr, "%d", currentRunningProcess.startTime);
                                sprintf(remainingTimeStr, "%d", currentRunningProcess.runningTime);

                                // Use these strings as arguments in execl
                                if (execl("./process.out", "process.out", startTimeStr, remainingTimeStr, NULL) == -1)
                                {
                                    perror("Can't open process.out! :(");
                                    exit(-1);
                                }
                            }
                        }

                        // If it has been forked before we continue the execution of the process
                        else
                        {
                            processResumingTime = getClk();
                            currentRunningProcess.waitTime += getClk() - currentRunningProcess.lastTimeActive;
                            fprintf(schedulerLog, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                            kill(currentRunningProcess.PID, SIGCONT);
                        }
                        printf("Scheduler: Process ID %d started at %d and has remaining time %d\n", currentRunningProcess.id, getClk(), currentRunningProcess.remainingTime);
                    }
                }
            }
            // Scheduler is idle, gets new process if exists
            else if (readyQueueHead != NULL)
            {
                if (isIdle)
                {
                    // We put the new process in the current running process
                    currentRunningProcess = peek(&readyQueueHead);
                    pop(&readyQueueHead);

                    // We check if it has been forked before
                    if (currentRunningProcess.PID == -1)
                    {
                        // Hasn't been forked before
                        // We update the start time, wait time and the PID of the process
                        processResumingTime = getClk();
                        currentRunningProcess.waitTime = getClk() - currentRunningProcess.arrivalTime;
                        fprintf(schedulerLog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                        currentPID = fork();
                        currentRunningProcess.startTime = getClk();
                        currentRunningProcess.PID = currentPID;
                        if (currentPID == 0)
                        {
                            char startTimeStr[20];
                            char remainingTimeStr[20];

                            sprintf(startTimeStr, "%d", currentRunningProcess.startTime);
                            sprintf(remainingTimeStr, "%d", currentRunningProcess.runningTime);

                            // Use these strings as arguments in execl
                            if (execl("./process.out", "process.out", startTimeStr, remainingTimeStr, NULL) == -1)
                            {
                                perror("Can't open process.out! :(");
                                exit(-1);
                            }
                        }
                    }
                    // If it has been forked before we continue the execution of the process
                    else
                    {
                        processResumingTime = getClk();
                        currentRunningProcess.waitTime += getClk() - currentRunningProcess.lastTimeActive;
                        fprintf(schedulerLog, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), currentRunningProcess.id, currentRunningProcess.arrivalTime, currentRunningProcess.runningTime, currentRunningProcess.remainingTime, currentRunningProcess.waitTime);
                        kill(currentRunningProcess.PID, SIGCONT);
                    }
                    printf("Scheduler: Process ID %d started at %d and has remaining time %d\n", currentRunningProcess.id, getClk(), currentRunningProcess.remainingTime);
                    isIdle = false;
                }
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////// Round Robin ////////////////////////////////////////
    else if (chosenAlgo == RR)
    {
        // Initialize the circular queue
        CircularQueue *circQueue = (CircularQueue *)malloc(sizeof(CircularQueue));
        initQueue(circQueue);

        // Get the quantum value
        int quantumValue = atoi(argv[3]);

        // The time when the process finished its quantum
        int currentFinishQuantum;

        // The current running process
        QNode *currRunningProc;
        Node *waitQueueHead = NULL;
        process currentWaitingProcess;
        bool check = true;

        // Keep working until recieved all proceses, no elements are left in the circular queue and the scheduler has finished all his processes
        while (numOfRecievedProcesses < totalNumOfProcesses || !isIdle || !isEmptyCirc(circQueue))
        {
            // Another while loop to recieve all processes that arrive at the same time before continuing
            while (numOfRecievedProcesses < totalNumOfProcesses)
            {
                // Recieve the process
                recVal = msgrcv(processMsgqID, &processJustRecieved, sizeof(processJustRecieved), 0, IPC_NOWAIT);

                // Check if a process was sent
                if (recVal != -1)
                {
                    printf("Recieved!\n");

                    // Increasing total running Time
                    totalRunningTime += processJustRecieved.procsAttributes[2];
                    // Put in ready queue

                    treeNode *found = allocateProcess(root, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[4]);
                    // process can fit in memory, so it is placed in readQueue
                    if (found != NULL)
                    {

                        QNode *newNode = newQNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], found->start, found->end);

                        enqueue(circQueue, newNode);

                        fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), processJustRecieved.procsAttributes[4], processJustRecieved.procsAttributes[0], found->start, found->end);
                        printTree(root);
                    }
                    else
                    {
                        // Put in waitQueue (ordered by arrival time)
                        if (waitQueueHead == NULL)
                        {
                            waitQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                        else
                        {
                            push(&waitQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[4], -1, -1);
                        }
                    }
                    numOfRecievedProcesses++;
                }
                // No processes were sent so break the loop
                else
                {
                    break;
                }
            }

            // If the scheduler is idle and there are processes in the ready queue
            if (!isEmptyCirc(circQueue) && isIdle)
            {
                // Get the process from the ready queue
                currRunningProc = dequeue(circQueue);
                int remTime = currRunningProc->proc.remainingTime;

                // runTime indicates whether the process will run a full quantum or its time remaining
                int runTime;
                if (remTime >= quantumValue)
                {
                    runTime = quantumValue;
                }
                else
                {
                    runTime = remTime;
                }

                // If the process hasn't been forked it we fork it
                if (currRunningProc->proc.PID == -1)
                {
                    currRunningProc->proc.waitTime = getClk() - currRunningProc->proc.arrivalTime;
                    fprintf(schedulerLog, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), currRunningProc->proc.id, currRunningProc->proc.arrivalTime, currRunningProc->proc.runningTime, currRunningProc->proc.remainingTime, currRunningProc->proc.waitTime);
                    currentPID = fork();
                    currRunningProc->proc.PID = currentPID;
                    currRunningProc->proc.startTime = getClk();
                    if (currentPID == 0)
                    {
                        char startTimeStr[20];
                        char remainingTimeStr[20];

                        sprintf(startTimeStr, "%d", currRunningProc->proc.startTime);
                        sprintf(remainingTimeStr, "%d", currRunningProc->proc.runningTime);

                        // Use these strings as arguments in execl
                        if (execl("./process.out", "process.out", startTimeStr, remainingTimeStr, NULL) == -1)
                        {
                            perror("Can't open process.out! :(");
                            exit(-1);
                        }
                    }
                }

                // If it has been forked before we continue the execution of the process
                else
                {
                    currRunningProc->proc.waitTime += getClk() - currRunningProc->proc.lastTimeActive;
                    kill(currRunningProc->proc.PID, SIGCONT);
                    fprintf(schedulerLog, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), currRunningProc->proc.id, currRunningProc->proc.arrivalTime, currRunningProc->proc.runningTime, currRunningProc->proc.remainingTime, currRunningProc->proc.waitTime);
                }
                // To indicate that the scheduler is currently running a process
                isIdle = false;
                printf("Scheduler: Process ID %d started at %d and has remaining time %d\n", currRunningProc->proc.id, getClk(), currRunningProc->proc.remainingTime);

                // To know when the process finishes its quantum
                currentFinishQuantum = getClk() + runTime;
                // Decrement the remaining time of the process
                currRunningProc->proc.remainingTime -= runTime;
            }
            // When the scheduler is not idle and the process has finished its quantum
            else if (getClk() == currentFinishQuantum && !isIdle)
            {
                // Reset the quantum finish time
                currentFinishQuantum = -1;

                // If the process hasn't finished its remaining time we pause it and enqueue it
                if (currRunningProc->proc.remainingTime > 0)
                {
                    currRunningProc->proc.lastTimeActive = getClk();
                    kill(currRunningProc->proc.PID, SIGSTOP);
                    fprintf(schedulerLog, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), currRunningProc->proc.id, currRunningProc->proc.arrivalTime, currRunningProc->proc.runningTime, currRunningProc->proc.remainingTime, currRunningProc->proc.waitTime);

                    // Checking for arrived process before enquing this process
                    while (numOfRecievedProcesses < totalNumOfProcesses)
                    {
                        // Recieve the process
                        recVal = msgrcv(processMsgqID, &processJustRecieved, sizeof(processJustRecieved), 0, IPC_NOWAIT);

                        // If recieved a process
                        if (recVal != -1)
                        {
                            printf("Recieved!\n");

                            // Increasing total running Time
                            totalRunningTime += processJustRecieved.procsAttributes[2];
                            // Put in ready queue

                            treeNode *found = allocateProcess(root, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[4]);
                            // printf("found  %d\n",found);
                            // process can fit in memory, so it is placed in readQueue
                            if (found != NULL)
                            {

                                QNode *newNode = newQNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[3], processJustRecieved.procsAttributes[4], found->start, found->end);

                                enqueue(circQueue, newNode);

                                fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), processJustRecieved.procsAttributes[4], processJustRecieved.procsAttributes[0], found->start, found->end);
                                printTree(root);
                            }
                            else
                            {
                                // Put in waitQueue (ordered by arrival time)
                                if (waitQueueHead == NULL)
                                {
                                    waitQueueHead = newNode(processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[4], -1, -1);
                                }
                                else
                                {
                                    push(&waitQueueHead, processJustRecieved.procsAttributes[0], processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[2], processJustRecieved.procsAttributes[2], -1, -1, -1, -1, -1, processJustRecieved.procsAttributes[1], processJustRecieved.procsAttributes[4], -1, -1);
                                }
                            }
                            numOfRecievedProcesses++;
                        }
                        // Not a valid process so break the recieving loop
                        else
                        {
                            break;
                        }
                    }
                    enqueue(circQueue, currRunningProc);
                }
                // If the process has finished its remaining time we print its data
                else
                {
                    printf("Scheduler: Process ID %d has finished at time %d byeeee\n", currRunningProc->proc.id, getClk());
                    deallocateProcess(root, currRunningProc->proc.id);
                    fprintf(memoryLog, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), currRunningProc->proc.memorySize, currRunningProc->proc.id, currRunningProc->proc.startAddress, currRunningProc->proc.endAddress);

                    // Waits for forked process to finish execution
                    // waitpid(currRunningProc->proc.PID, NULL, 0);
                    while (waitQueueHead != NULL)
                    {
                        currentWaitingProcess = peek(&waitQueueHead);
                        pop(&waitQueueHead);
                        treeNode *found = allocateProcess(root, currentWaitingProcess.id, currentWaitingProcess.memorySize); 
                        printTree(root);
                        if (found != NULL)
                        {

                            currentWaitingProcess.startAddress = found->start;
                            currentWaitingProcess.endAddress = found->end;
                            QNode *newNode = newQNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.remainingTime, currentWaitingProcess.memorySize, found->start, found->end);
                            enqueue(circQueue, newNode);

                            fprintf(memoryLog, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(), currentWaitingProcess.memorySize, currentWaitingProcess.id, currentWaitingProcess.startAddress, currentWaitingProcess.endAddress);
                        }
                        else
                        {
                            if (waitQueueHead == NULL)
                            {
                                waitQueueHead = newNode(currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.arrivalTime, currentWaitingProcess.memorySize, -1, -1);
                            }
                            else
                            {
                                push(&waitQueueHead, currentWaitingProcess.id, currentWaitingProcess.arrivalTime, currentWaitingProcess.runningTime, currentWaitingProcess.remainingTime, -1, -1, -1, -1, -1, currentWaitingProcess.arrivalTime, currentWaitingProcess.memorySize, -1, -1);
                            }
                            break;
                        }
                    }
                    // Calculating Statistics for output file
                    float weightedTurnAroundTime = (float)(getClk() - currRunningProc->proc.arrivalTime) / currRunningProc->proc.runningTime;
                    totalWeightedTurnAroundTime += weightedTurnAroundTime;
                    totalWaitingTime += currRunningProc->proc.waitTime;
                    weightedTurnAroundTimes[numOfProcessesFinished] = weightedTurnAroundTime;
                    numOfProcessesFinished++;
                    fprintf(schedulerLog, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), currRunningProc->proc.id, currRunningProc->proc.arrivalTime, currRunningProc->proc.runningTime, currRunningProc->proc.remainingTime, currRunningProc->proc.waitTime, getClk() - currRunningProc->proc.arrivalTime, weightedTurnAroundTime);
                }

                isIdle = true;
            }
        }
        // Free the circular queue
        free(circQueue);
    }

    ///////////////////////////////////// Writing in .perf File /////////////////////////////////////

    // Getting finish time
    int schedulerFinishTime = getClk() - 1;

    // Calculating statistics
    fprintf(schedulerPerf, "CPU utilization = %.2f%%\n", (float)totalRunningTime / schedulerFinishTime * 100);
    fprintf(schedulerPerf, "Avg WTA = %.2f\n", totalWeightedTurnAroundTime / totalNumOfProcesses);
    fprintf(schedulerPerf, "Avg Waiting = %.2f\n", (float)totalWaitingTime / totalNumOfProcesses);
    float averageWeightedTurnAroundTime = totalWeightedTurnAroundTime / totalNumOfProcesses;

    // Calculating variance and STD
    float variance = 0;
    for (int i = 0; i < totalNumOfProcesses; i++)
    {
        variance += (weightedTurnAroundTimes[i] - averageWeightedTurnAroundTime) * (weightedTurnAroundTimes[i] - averageWeightedTurnAroundTime);
    }
    fprintf(schedulerPerf, "Std WTA = %.2f\n", sqrt(variance / totalNumOfProcesses));

    // Closing all the files and destroying the clock
    fclose(schedulerLog);
    fclose(schedulerPerf);
    fclose(memoryLog);

    //  Freeing the array storing the weighted turn around times
    free(weightedTurnAroundTimes);
    destroyClk(false);

    return 0;
}