#include <stdlib.h>

#include "multilevelQueueScheduler.h"

void attemptPromote( schedule *ps );
int min( int x, int y );

static const int STEPS_TO_PROMOTION = 50;
static const int FOREGROUND_QUEUE_STEPS = 5;

/*static process* unfinishedProc = NULL;*/
static process* tempProc = NULL;
static bool promoted = false, promotionImminent = false;
#define pass (void)0

/* printNames
 * input: none
 * output: none
 *
 * Prints names of the students who worked on this solution
 */
void printNames( )
{
    /* TODO : Fill in you and your partner's names (or N/A if you worked individually) */
    printf("\nThis solution was completed by:\n");
    printf("Christian Walker\n\n");
}

/* createSchedule
 * input: none
 * output: a schedule
 *
 * Creates and return a schedule struct.
 */
schedule* createSchedule( ) {
    
    /* TODO: malloc space for a new schedule and initialize the data in it */
    schedule* s = (schedule*)malloc(sizeof(schedule));
    s->foreQueue = createQueue();
    s->backQueue = createQueue();
    s->stepCount = 0;
    
    return s; /* TODO: Replace with your return value */
}

/* isScheduleUnfinished
 * input: a schedule
 * output: bool (true or false)
 *
 * Check if there are any processes still in the queues.
 * Return TRUE if there is.  Otherwise false.
 */
bool isScheduleUnfinished( schedule *ps ) {
    /* TODO: check if there are any process still in a queue.  Return TRUE if there is. */
    if(!isEmpty(ps->foreQueue) || !isEmpty(ps->backQueue)){
        return true;
    }
    else{
        return false;
    }
}

/* addNewProcessToSchedule
 * input: a schedule, a string, a priorityity
 * output: void
 *
 * Create a new process with the provided name and priorityity.
 * Add that process to the appropriate queue
 */
void addNewProcessToSchedule( schedule *ps, char *processName, priority p ) {
    processData* pD = initializeProcessData(processName);
    process* proc = (process*)malloc(sizeof(process));
    proc->priority = p;
    proc->name = processName;
    proc->data = pD;
    proc->stepAdded = (ps->stepCount);
    proc->stepsRan = 5;
    
    if(p == FOREGROUND){
        enqueue(ps->foreQueue, proc);
    }
    else{
        enqueue(ps->backQueue, proc);
    }
}

/* runNextProcessInSchedule
 * input: a schedule
 * output: a string
 *
 * Use the schedule to determine the next process to run and for how many time steps.
 * Call "runProcess" to attempt to run the process.  You do not need to print anything.
 * You should return the string received from "runProcess".  You do not need to use/modify this string in any way.
 */
char* runNextProcessInSchedule( schedule *ps ) {
    /*prevExecuted++;*/
    /* TODO: complete this function.
    The function "runProcess", "loadProcessData", and "freeProcessData"
    in processSimulator.c will be useful in completing this.*/
    char *ret = NULL;
    int numSteps = 0, localRan;
    bool b;
    
    /*EDIT: promotionImminent is a global boolean that evaluates true if there will be a promotion next turn.
    * this promotes if there is meant to be a promotion.*/
    if(promotionImminent){
        attemptPromote(ps);
        promotionImminent = false;
    }
    
    /*EDIT: removed "|| promoted", which would make this statement always evaluate true for the case at 187. This works because
    * there's an edit to "stepsRan" later on.*/
    if((tempProc != NULL && tempProc->priority == FOREGROUND && tempProc->stepsRan != 5 && !(isEmpty(ps->foreQueue)))){
        numSteps = tempProc->stepsRan;
    }
    else{
        attemptPromote( ps );
        
        if( !isEmpty(ps->foreQueue) ){
            tempProc = getNext(ps->foreQueue);
            
            if(isEmpty(ps->backQueue)){
                if(tempProc->stepsRan != FOREGROUND_QUEUE_STEPS){
                    numSteps = tempProc->stepsRan;
                }
                else{
                    numSteps = FOREGROUND_QUEUE_STEPS;
                }
            }
            else{
                numSteps = min((getNext(ps->backQueue)->stepAdded + STEPS_TO_PROMOTION ) - ps->stepCount, FOREGROUND_QUEUE_STEPS);
            }
        }
        else if( !isEmpty(ps->backQueue) ){
            tempProc = getNext(ps->backQueue);
            numSteps = (tempProc->stepAdded + STEPS_TO_PROMOTION ) - ps->stepCount;
        }
    }
    
    localRan = numSteps;
    loadProcessData(tempProc->data);
    b = runProcess( tempProc->name, &ret, &localRan );
    
    if(b){
        tempProc = (tempProc->priority == FOREGROUND) ? dequeue(ps->foreQueue) : dequeue(ps->backQueue);
        freeProcessData();
        free(tempProc->name);
        free(tempProc);
        if(tempProc != NULL){
            tempProc = NULL;
        }
    }
    else if(tempProc->priority == BACKGROUND) pass;
    else{
        if(localRan == numSteps){
            if( !isEmpty(ps->backQueue) && (((getNext(ps->backQueue)->stepAdded + STEPS_TO_PROMOTION) - (ps->stepCount + localRan)) == 0)){
                promotionImminent = true;
                /*EDIT: This is the change I mentioned at the top. In this situation, "stepsRan" has kind of become "stepsLeft".
                * In the case that the process was interrupted, we need to save how many steps are left to run. Instead of subtracting
                * the localRan from 5, it should be the steps the process has already executed, because what is the guarantee that
                * it was supposed to run for 5?*/
                tempProc->stepsRan -= localRan;
                /*EDIT: The problem with 73 was that it was evaulating as 0 steps left. There was no check that there were any steps
                * left to do. This checks if it's done with its turn and acts accordingly.*/
                if(tempProc->stepsRan == 0){
                    enqueue(ps->foreQueue, dequeue(ps->foreQueue));
                    tempProc = NULL;
                }
            }
            else{
                tempProc->stepsRan = 5;
                enqueue(ps->foreQueue, dequeue(ps->foreQueue));
            }
        }
        else{
            tempProc->stepsRan = numSteps - localRan;
        }
    }
    
    ps->stepCount += localRan;
    
    promoted = false;
    return ret;
}

/* attemptPromote
 * input: a schedule
 * output: none
 *
 * Promote every background process that has been waiting for 50 time steps.
 * This function might be tricky so you might save it for last.
 */
void attemptPromote( schedule *ps ) {
    /* TODO: complete this function.
    The function "promoteProcess" in processSimulator.c will be useful in completing this. */
    int count = 0;
    
    do{
        if(isEmpty(ps->backQueue)){
            return;
        }
        if((getNext(ps->backQueue))->stepAdded + STEPS_TO_PROMOTION  - (ps->stepCount) == 0){
            promoteProcess(getNext(ps->backQueue)->name, getNext(ps->backQueue)->data);
            getNext(ps->backQueue)->priority = FOREGROUND;
            enqueue(ps->foreQueue, dequeue(ps->backQueue));
            promoted = true;
        }
        else{
            count++;
        }
    }while(count == 0);
}
/* freeSchedule
 * input: a schedule
 * output: none
 *
 * Free all of the memory associated with the schedule.
 */
void freeSchedule( schedule *ps ) {
    if(isEmpty(ps->foreQueue)){
        freeQueue(ps->foreQueue);
    }
    else{
        printf("\nSOMETHING WENT WRONG: FOREQUEUE ISNT EMPTY\n");
        exit(-1);
    }
    
    if(isEmpty(ps->backQueue)){
        freeQueue(ps->backQueue);
    }
    else{
        printf("\nSOMETHING WENT WRONG: BACKQUEUE ISNT EMPTY\n");
        exit(-1);
    }
    
    free(ps);
    
}

/* PROVIDED FUNCTION
 * freeSchedule
 * input: two ints
 * output: the smaller of the 2 ints
 */
int min( int x, int y ){
    if( x<y )
        return x;
    return y;
}
