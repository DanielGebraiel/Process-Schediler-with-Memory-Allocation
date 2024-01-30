#include "headers.h"

int newStartingPoint;

void contHandler(int signum);

int main(int agrc, char *argv[])
{
    signal(SIGCONT, contHandler);

    // Connect the process to the Clock
    initClk();

    // Get the inputs from the scheduler
    int startTime = atoi(argv[1]);
    int runningTime = atoi(argv[2]);

    // This variable indicates the starting point after a clock step
    newStartingPoint = startTime;

    // Remaining time of process
    int remainingTime = runningTime;

    while (remainingTime > 0)
    {
        // If we have reached the next time step
        if (getClk() - newStartingPoint == 1)
        {
            // Decement the remaining time
            remainingTime--;

            // Update the current time step
            newStartingPoint = getClk();
        }
    }

    // Updates on terminal when the process has finished
    printf("Process: Process has finished at %d!\n", getClk());
    destroyClk(false);

    return 0;
}

void contHandler(int signum)
{
    newStartingPoint = getClk();
}