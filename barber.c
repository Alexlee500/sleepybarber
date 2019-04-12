/*
*   Alexander Lee
*   #989222904
*   COMP173
*   Lab 6: Sleeping Barber
*
*   gcc -pthread -o barber barber.c
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

void *barber(void *);
void *customer(int *count);
void cutHair();

int numChairs = 5;
int currentCustomer;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t customers;
sem_t chairs;
sem_t barbers;

int main(int argc, char *argv[])
{
  //checking argument for validity
  if (argc != 2 || atoi(argv[1]) < 1)
  {
    printf("Usage: %s <number_of_chairs>\n", argv[0]);
    return -1;
  }

  //initialize random number generator
  srand(time(NULL));

  //set number of chairs to argument
  numChairs = atoi(argv[1]);

  sem_init(&customers, 0, 0); //semaphore counter for the number of customers waiting for a haircut
  sem_init(&chairs, 0, numChairs); //semaphore counter for number of empty chairs
  sem_init(&barbers, 0, 0); //Semaphore counter for available barbers to cut hair


  // Create thread for the barber
  pthread_t tidBarber;
  pthread_attr_t attrBarber;
  pthread_attr_init(&attrBarber);
  pthread_create(&tidBarber, &attrBarber, barber, NULL);

  // counter for customers
  int customerCount = 1;

  //create have a new customer arrive at set intervals
  //new customer arrives every 2-10 seconds
  int timedelay = 5;
  int delayMin = 2;
  int delayMax = 10;
  while (1)
  {
    //random value between in a range
    timedelay = rand() % (delayMax + 1 - delayMin) + delayMin;

    pthread_t tidCustomer;
    pthread_attr_t attrCustomer;
    pthread_attr_init(&attrCustomer);
    pthread_create(&tidCustomer, &attrCustomer, customer, customerCount);
    customerCount = customerCount + 1;
    sleep (timedelay);
  }

  // Sleep main
  sleep(-1);
  return 0;
}


//Barber function
void *barber(void *trash)
{
  printf ("Opened barber shop with %d chairs\n", numChairs);
  while(1)
  {
      //wait for customers to be greater than 0.
      //Semiphore decrement by 1
      sem_wait(&customers);

      //lock mutex
      pthread_mutex_lock(&mutex);

      // increase number of empty chairs by 1
      sem_post(&chairs);
      //increase number of barbers by 1
      sem_post(&barbers);
      //release mutex
      pthread_mutex_unlock(&mutex);
      cutHair();
  }
}

//customer function
void *customer(int *count)
{
  printf("Customer %d enters barber shop\n", count);

  //Critical Area. Lock Mutex
  pthread_mutex_lock(&mutex);

  //if there are empty chairs, decrement number of empty by 1
  if (sem_trywait(&chairs) == 0)
  {
    //increase number of customers by 1
    sem_post(&customers);

    //printing semaphore values for empty chairs
    int emptyChairs;
    sem_getvalue(&chairs, &emptyChairs);
    printf("\tCustomer %d sits in a chair. There are now %d empty chairs\n", count, emptyChairs);

    // end critical area. Unlock Mutex
    pthread_mutex_unlock(&mutex);

    // Decrease number of barbers by 1
    sem_wait(&barbers);

    // Reprint values
    sem_getvalue(&chairs, &emptyChairs);
    printf("\tCustomer %d vacates a chair and goes to get their hair cut. There are now %d empty chairs\n", count, emptyChairs);

    //set the value of the current customer getting a haircut
    currentCustomer = count;
  }

  // Leave barber shop if there are no empty chairs
  else
  {
    printf("Customer %d leaves the barber shop\n", count);
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    return 0;
  }

}

//cut hair.
void cutHair()
{
  //generate random time for haircut
  //haircut takes betweek 5-15 secodns
  int cutTimeMin = 5;
  int cutTimeMax = 15;
  int cutTime = rand() % (cutTimeMax + 1 - cutTimeMin) + cutTimeMin;

  sleep(cutTime);
  printf("Customer %d finished getting their hair cut after %d seconds and leaves the barber shop\n",currentCustomer, cutTime);
}
