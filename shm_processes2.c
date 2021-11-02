/* CSCI 401: Operating Systems: Lab #3, Part 2
 * Topic: Processes & Shared Memory 
 * Subtopic: Communication between Parent and Child Process via Shared Memory
 * 
 * Programmer (Student ID): Ben Corriette (@02956064)
 * Last modified date: 11/02/2021
 * 
 * References: https://www.tutorialspoint.com/generating-random-number-in-a-range-in-c
 *             https://stackoverflow.com/questions/28457525/how-do-you-kill-zombie-process-using-wait
 *             https://stackoverflow.com/questions/1686045/strict-alternation-in-the-c-programming-language-from-tanenbaum
 *             ./shm_processes.c
 *             Linux Programmer's Manual
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

int account = 0;
int parent_iteration_count = 0;
int child_iteration_count = 0;

const int MAX_ACCT_BALANCE = 100;
// Req. #5: Both parent and child processes should loop 25 times
const int MAX_ITERATIONS = 25; 

// Generate a random number within a range
int generate_random_number(int upper, int lower) {
    return (rand() % (upper - lower + 1)) + 1;
}

// Req. #3: Allow the parent process to deposit money
void deposit_money(int shm_memory[], int bank_acct) {
  int random_amount = generate_random_number(0, MAX_ACCT_BALANCE);
  
  // If random number is even, deposit the amount into the account
  if (random_amount % 2 == 0) {
    bank_acct += random_amount;
    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", random_amount, bank_acct);
  }
  else {
    printf("Dear old Dad: Doesn't have any money to give.\n");
  }
  
  shm_memory[0] = bank_acct;
}

// Req. #4: Allow the child process to withdraw money
void withdraw_money(int shm_memory[], int balance, int account) {
  account -= balance;
  printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
  
  shm_memory[0] = account;
}


int main()
{
  int shm_id;
  int *shm_ptr;
  pid_t process_id;
  int child_status;
  int errsv;
  
  // Req. #1: Create two variables initialized to zero
  int bank_account = 0;
  int turn = 0;
  
  // Create shared memory segment
  shm_id = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
  if (shm_id == -1) {
      errsv = errno;
      printf("Shared memory segment failed to be created; Error # %d\n", errsv);
      exit(EXIT_FAILURE);
  }
  printf("Parent process has received a shared memory of two integers...\n");

  // Attach shared memory segment to suitable memory address
  shm_ptr = (int *) shmat(shm_id, NULL, 0);
  if (*shm_ptr == -1) {
      errsv = errno;
      printf("Shared memory segment failed to attach to memory address; Error # %d\n", errsv);
      exit(EXIT_FAILURE);
  }
  printf("Parent process has attached the shared memory...\n");

  shm_ptr[0] = bank_account;
  shm_ptr[1] = turn;
  
  // Req. #2: Create two processes (parent and child)
  printf("Parent process is about to fork a child process...\n");
  process_id = fork();
	
  if (process_id == -1) {
      errsv = errno;
      printf("Child process failed to be forked off from parent; Error # %d\n", errsv);
      exit(EXIT_FAILURE);
  }
  else {
    srand(time(0));
  }
   
  // Handle parent and child processes
  if (process_id == 0) {
    int random_number = generate_random_number(0, 5);
		 
    while (child_iteration_count < MAX_ITERATIONS) {
      while (shm_ptr[1] != 1) {
        // Do no-op
      }
      int random_balance = generate_random_number(0, 50);
			child_iteration_count++;
			printf("Starting child process loop # %d of %d\n", child_iteration_count, MAX_ITERATIONS);

			sleep(random_number);
			account = shm_ptr[0];
      printf("Poor Student needs $%d\n", random_balance);

      if (random_balance <= account) {
        withdraw_money(shm_ptr, random_balance, account);
      }
      else {
        printf("Poor Student: Not Enough Cash ($%d)\n", account);
      }
			
			shm_ptr[1] = 0;
    }
    printf("Child process exits...\n");
    exit(EXIT_SUCCESS);
  }
  else if (process_id > 0) {  
    int random_number = generate_random_number(0, 5);
    
    while (parent_iteration_count < MAX_ITERATIONS) {
      while (shm_ptr[1] != 0) {
        // Do no-op
      }
			parent_iteration_count++;
			printf("Parent process loop # %d of %d\n", parent_iteration_count, MAX_ITERATIONS);
			sleep(random_number);
			account = shm_ptr[0];
      
			if (account <= MAX_ACCT_BALANCE) {
        deposit_money(shm_ptr, account);
      }
      else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
			}
			
			shm_ptr[1] = 1;
    }  
    printf("Parent process exits...\n");
    exit(EXIT_SUCCESS);
  }
	if (wait(&child_status) == -1) {
      errsv = errno;
      printf("Child process failed to return status to parent; Error # %d\n", errsv);
      exit(EXIT_FAILURE);
  }
	printf("Parent process has detected the completion of its child...\n");

	
	if (shmdt((void *) shm_ptr) == -1) {
		errsv = errno;
		printf("Shared memory failed to detach from memory address; Error # %d\n", errsv);
		exit(EXIT_FAILURE);
	}
	printf("Parent process has detached its shared memory...\n");
    
	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
      errsv = errno;
      printf("Shared memory segment failed to be destroyed; Error # %d\n", errsv);
      exit(EXIT_FAILURE);
	}
	printf("Parent process has removed its shared memory...\n");
	exit(EXIT_SUCCESS);
}
