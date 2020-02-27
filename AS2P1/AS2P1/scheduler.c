#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "util.h"
#include <time.h>
// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

#define run_state 0
#define exit_state 1
#define sleep_state 2
#define wait_state 3

// This struct will hold the all the necessary information for each task
typedef struct task_info {
  // This field stores all the state required to switch back to this task
  ucontext_t context;
  
  // This field stores another context. This one is only used when the task
  // is exiting.
  ucontext_t exit_context;
  
  // TODO: Add fields here so you can:
  //   a. Keep track of this task's state.
  int task_state;
  //   b. If the task is sleeping, when should it wake up?
  size_t wakeup_time;
  //   c. If the task is waiting for another task, which task is it waiting for?
  int wait_for_task[MAX_TASKS];
  //   d. Was the task blocked waiting for user input? Once you successfully
  //      read input, you will need to save it here so it can be returned.
  char input;
} task_info_t;

int current_task = 0; //< The handle of the currently-executing task
int num_tasks = 1;    //< The number of tasks created so far
task_info_t tasks[MAX_TASKS]; //< Information for every task


void signalHandler(int sign){
    if(sign ==SIGALRM){
            int i =1;
            for (i =1; i <num_tasks; i++){
                if(tasks[i].task_state==sleep_state){
                    if(tasks[i].wakeup_time<= time_ms()){
                        tasks[i].task_state= run_state;
                        //printf("   Wakeup SIGALRM signal! task[%d] wake state: %d\n", i,tasks[i].task_state );
//                        printf("   task[%d] wakeup at %zu \n", i, time_ms());
                        
                        int prev_task = current_task;
                        current_task = i;
//                        printf("   wakeup:swapcontext(&tasks[%d].context, &tasks[%d].context) \n", prev_task, current_task);
                        swapcontext(&tasks[prev_task].context, &tasks[current_task].context);
       
                         
                    }
                }
            }
    }
}
/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functiosn in this file.
 */
void scheduler_init() {
  // TODO: Initialize the state of the scheduler
    current_task = 0;
    tasks[0].task_state = run_state;
    int i;
    for(i =1; i<MAX_TASKS; i++)
        tasks[0].wait_for_task[i]=0 ;
    getcontext(&tasks[0].context);
    // Allocate a stack for the new task and add it to the context
    tasks[0].context.uc_stack.ss_sp = malloc(STACK_SIZE);
    tasks[0].context.uc_stack.ss_size = STACK_SIZE;
   /*
    signal(SIGALRM, signalHandler);
    struct itimerval new_value, old_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_usec = 1;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_usec = 200000;
    setitimer (ITIMER_REAL, &new_value, &old_value);
     */
}


/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {
  // TODO: Handle the end of a task's execution here
    tasks[current_task].task_state = exit_state;
    tasks[0].wait_for_task[current_task] =0;
    
    // check main's waiting condition
    waiting_check();
    
    printf("    tasks[%d] exit\n", current_task);
    print_wait_arr();
    
    swapcontext(&tasks[current_task].context,&tasks[round_robin_next()].context);
}

// check main's waiting condition
void waiting_check(){
    int i;
    for(i=1; i<num_tasks; i++){
        if (tasks[0].wait_for_task[i] == 1){
            tasks[0].task_state = wait_state;
            break;
        }
        else
            tasks[0].task_state = run_state;
    }
}
/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
  // Claim an index for the new task
  int index = num_tasks;
  num_tasks++;
  
  // Set the task handle to this index, since task_t is just an int
  *handle = index;
    
    
  tasks[index].task_state = run_state;
 
  // We're going to make two contexts: one to run the task, and one that runs at the end of the task so we can clean up. Start with the second
  
  // First, duplicate the current context as a starting point
  getcontext(&tasks[index].exit_context);
  
  // Set up a stack for the exit context
  tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;
  
  // Set up a context to run when the task function returns. This should call task_exit.
  makecontext(&tasks[index].exit_context, task_exit, 0);
  
  // Now we start with the task's actual running context
  getcontext(&tasks[index].context);
  
  // Allocate a stack for the new task and add it to the context
  tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].context.uc_stack.ss_size = STACK_SIZE;
  
  // Now set the uc_link field, which sets things up so our task will go to the exit context when the task function finishes
  tasks[index].context.uc_link = &tasks[index].exit_context;
  
  // And finally, set up the context to execute the task function
  makecontext(&tasks[index].context, fn, 0);
}

/**
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
  // TODO: Block this task until the specified task has exited.
  
  // start
    int prev_task = current_task;
    current_task = handle;
    
    //once call a wait, the main task will set state to wait and the bit of wait array to 1, as long as wainting state, the main will not be picked to run.
    //if handle is alreay exit, then there is no need to wait
    if(tasks[handle].task_state!=exit_state){
        tasks[prev_task].task_state = wait_state;
        tasks[prev_task].wait_for_task[current_task] = 1;
    }
    
    printf("    tasks[0].wait_for_task: %d \n", current_task);
    print_wait_arr();
    
    //printf("swapcontext(&tasks[%d].context, &tasks[%d].context)\n", prev_task, current_task);
    if(tasks[current_task].task_state== run_state)
        swapcontext(&tasks[prev_task].context, &tasks[current_task].context);
    else{
        current_task =round_robin_next();
        swapcontext(&tasks[prev_task].context, &tasks[current_task].context);
    }
}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 * 
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
  // TODO: Block this task until the requested time has elapsed.
  // Hint: Record the time the task should wake up instead of the time left for it to sleep. The bookkeeping is easier this way.
    if(ms>0){
        //swapcontext(&tasks[current_task].context, &tasks[current_task+1].context);
        //current_task +=1;
        int prev_task = current_task;
        tasks[current_task].task_state=sleep_state;
        
        tasks[current_task].wakeup_time = time_ms()+ ms;
        //tasks[0].wait_for_task ++;
        //sleep_ms(ms);
 //       printf("   task[%d] sleeps at %zu \n", current_task, time_ms());
        current_task = round_robin_next();
//        printf("   %d sleep:swapcontext(&tasks[%d].context, &tasks[%d].context) \n", prev_task, prev_task, current_task);
        swapcontext(&tasks[prev_task].context, &tasks[current_task].context);

        
        
    }
}


/**
* In Round-robin fashion choose next task to run
* Choose next task in the arry, if its state is "run" and not waiting for others then run it
* Otherwise iterate to next task, do the same check to decide run it or not.
* return the id of task to run
*/
int round_robin_next(){
    int temp_current_task = current_task;
    temp_current_task++;
    if(temp_current_task > num_tasks-1)
        temp_current_task = 0;
    /*
    printf("    num_tasks: %d \n",num_tasks);
    printf("    task0_state: %d wait: %d\n",tasks[0].task_state, tasks[0].wait_for_task);
    printf("    task1_state: %d wait: %d\n",tasks[1].task_state, tasks[1].wait_for_task);
    printf("    task2_state: %d wait: %d\n",tasks[2].task_state, tasks[2].wait_for_task);
    printf("    current_task: %d\n",current_task);
     */
    while (tasks[temp_current_task].task_state!=run_state){
        temp_current_task++;
        if(temp_current_task > num_tasks-1){
            temp_current_task = 0;
        }
    }
//    printf("    next run: %d \n", temp_current_task);
    current_task = temp_current_task;
    return current_task;
}

// print main's waiting array
void print_wait_arr(){
    printf("    Main's wait: ");
    int i;
    for(i = 1 ;i < num_tasks; i++)
    {
        printf("%d ",tasks[0].wait_for_task[i]);
    }
    printf("\n");
}
/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
  // TODO: Block this task until there is input available.
  // To check for input, call getch(). If it returns ERR, no input was available.
  // Otherwise, getch() will returns the character code that was read.
  int ch;
  if((ch = getch()))
      return ch;
  return ERR;
}
