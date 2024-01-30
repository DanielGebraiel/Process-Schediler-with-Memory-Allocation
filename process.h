
typedef struct process
{
    int id;
    int arrivalTime;
    int priority;
    int runningTime;
    int remainingTime;
    int waitTime;
    int startTime;
    int finishTime;
    int PID;
    int lastTimeActive;
    int memorySize;
    int startAddress;
    int endAddress;
} process;

typedef struct processBuff
{
    long ptype;
    int procsAttributes[5];
} processBuff;
