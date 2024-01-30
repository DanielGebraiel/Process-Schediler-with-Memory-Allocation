#include "headers.h"

void clearResources(int);

int processMsgqID;
int main(int argc, char *argv[])
{
    // Attatching signal handler to clear resources
    signal(SIGINT, clearResources);

    ///////////////////////////// 1. Read the input files. /////////////////////////////
    FILE *inputFile;
    inputFile = fopen("processes.txt", "r");
    if (inputFile == NULL)
    {
        perror("Cant open file :(");
        exit(0);
    }
    int id, arrival, runtime, priority, memSize;
    // Variable name to get number of elements in line
    int readCount;
    // Number of processes read from file
    int countProcs;

    // Keep reading until end of file
    while ((readCount = fscanf(inputFile, "%d %d %d %d %d", &id, &arrival, &runtime, &priority, &memSize)) != EOF)
    {
        // If the line has 5 elements, then it's a valid process
        if (readCount == 5)
        {
            countProcs++;
        }
        else
        {
            // If the line has less than 5 elements, then it's invalid
            int skippedChar;
            // Skip the rest of the line
            while (skippedChar = (fgetc(inputFile)) != '\n' && skippedChar != EOF)
            {
            }
        }
    }

    // Create an array of processes with the size of the number of processes read from file
    process *procsArray = (process *)malloc(countProcs * sizeof(process));

    // Reset the file pointer to the beginning of the file
    fclose(inputFile);
    inputFile = fopen("processes.txt", "r");

    // Read the file again and fill the array with the processes
    int currProcess = 0;
    while ((readCount = fscanf(inputFile, "%d %d %d %d %d", &id, &arrival, &runtime, &priority, &memSize)) != EOF)
    {
        if (readCount == 5)
        {
            process process;
            process.id = id;
            process.arrivalTime = arrival;
            process.runningTime = runtime;
            process.priority = priority;
            process.remainingTime = runtime;
            process.waitTime = 0;
            process.finishTime = -1; // to be changed later if needed (farida)
            process.memorySize = memSize;

            procsArray[currProcess] = process;

            currProcess++;
        }
        else
        {
            int skippedChar;
            while (skippedChar = (fgetc(inputFile)) != '\n' && skippedChar != EOF)
            {
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////

    /////////////////////// 2. Get chosen Algorithm and Quantum. ///////////////////////
    int chosenAlgo;
    int quantumRR = 0;

    printf("Please choose an algorithm:\n 1: HPF\n 2: SRTN\n 3: Round Robin\n");
    scanf("%d", &chosenAlgo);

    // Validate input
    while (chosenAlgo < 1 || chosenAlgo > 3)
    {
        printf("Invalid input!!\n");
        printf("Please choose an algorithm:\n 1: HPF\n 2: SRTN\n 3: Round Robin\n");
        scanf("%d", &chosenAlgo);
    }
    // Incase of Round Robin, get the quantum
    if (chosenAlgo == 3)
    {
        printf("Enter Quantum for Round Robin: ");
        scanf("%d", &quantumRR);
    }

    ////////////////////////////////////////////////////////////////////////////////////

    ///////////// 3. Initiate and create the scheduler and clock processes./////////////

    pid_t clockPid = fork();
    if (clockPid == -1)
    {
        perror("Cannot fork to make clock! :(");
        exit(-1);
    }
    // Incase of child process: fork to make the clock
    else if (clockPid == 0)
    {
        if (execl("./clk.out", "clk.out", NULL) == -1)
        {
            perror("Can't open clk.out! :(");
            exit(-1);
        }
    }
    // Incase of parent process: fork again to make the scheduler
    else
    {
        pid_t schedulerPid = fork();
        if (schedulerPid == -1)
        {
            perror("Cannot fork to make scheduler! :(");
            exit(-1);
        }
        else if (schedulerPid == 0)
        {
            // Convert the number of processes, chosen algorithm and quantum to strings
            char countProcsStr[20];
            char chosenAlgoStr[20];
            char quantumRRStr[20];

            sprintf(countProcsStr, "%d", countProcs);
            sprintf(chosenAlgoStr, "%d", chosenAlgo);
            sprintf(quantumRRStr, "%d", quantumRR);

            // Use these strings as arguments in execl
            if (execl("./scheduler.out", "scheduler.out", countProcsStr, chosenAlgoStr, quantumRRStr, NULL) == -1)
            {
                perror("Can't open scheduler.out! :(");
                exit(-1);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////// 4. Initialize the Clock. /////////////////////////////

    initClk();

    ////////////////////////////////////////////////////////////////////////////////////

    ///////////////////// 5. Send the processes to the scheduler ///////////////////////

    // Keep track of the number of processes sent
    int numOfProcsSent = 0;
    // Create the message queue
    processMsgqID = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (processMsgqID == -1)
    {
        perror("Error in creating process message queue! :(");
        exit(-1);
    }

    // Create a buffer to send the processes
    processBuff processToBeSent;

    // Send the processes to the scheduler
    while (numOfProcsSent < countProcs)
    {
        // If the current time is equal to the arrival time of the process, send it
        if (getClk() == procsArray[numOfProcsSent].arrivalTime)
        {
            // Sending to scheduler
            processToBeSent.ptype = 1;
            processToBeSent.procsAttributes[0] = procsArray[numOfProcsSent].id;
            processToBeSent.procsAttributes[1] = procsArray[numOfProcsSent].arrivalTime;
            processToBeSent.procsAttributes[2] = procsArray[numOfProcsSent].runningTime;
            processToBeSent.procsAttributes[3] = procsArray[numOfProcsSent].priority;
            processToBeSent.procsAttributes[4] = procsArray[numOfProcsSent].memorySize;

            int sendVal = msgsnd(processMsgqID, &processToBeSent, sizeof(processBuff), !IPC_NOWAIT);
            if (sendVal == -1)
            {
                perror("Errror in sending! :(");
                exit(-1);
            }
            printf("Sent!\n");
            numOfProcsSent++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////

    //////////// 6. Wait for the scheduler and clean resources when finished. //////////
    wait(NULL);

    free(procsArray);

    if (msgctl(processMsgqID, IPC_RMID, NULL) == -1)
    {
        perror("Error in destroying message queue in process generator!! :( \n");
        exit(-1);
    }
    destroyClk(true);

    return 0;
}

void clearResources(int signum)
{
    // Check if the message queue exists
    int processMsgqID = msgget(MSGKEY, 0666 | IPC_CREAT);

    // If it exists, destroy it
    if (msgctl(processMsgqID, IPC_RMID, NULL) == -1)
    {
        perror("Error in destroying message queue!! :( \n");
        exit(-1);
    }
    destroyClk(true);
    exit(0);
}
