/* 
    File: simpleclient.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2013/01/31

    Simple client main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define NUMBER_OF_REQUEST_AND_STATISTICS_THREADS  3

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>

#include "reqchannel.H"
#include "semaphore.H"
#include "bounded_buffer.H"


using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* refers to the three users on the server, see request_channel_data_input */
vector< vector<int> > server_responses(NUMBER_OF_REQUEST_AND_STATISTICS_THREADS);

/*--------------------------------------------------------------------------*/
/* CONSTANTS, INITIAL VARIABLES */
/*--------------------------------------------------------------------------*/

int number_of_data_requests = 10;
int data_requests_produced_size_joe = 0;
int data_requests_produced_size_jane= 0;
int data_requests_produced_size_john= 0;

int data_requests_consumed_size = 0; 
int histogram_size = {0, 0, 0};
int size_of_bounded_buffer = 1;
int number_of_worker_threads = 1;
bool timer = false;

char* people[] = {"Joe Smith", "Jane Smith", "John Doe"};

Bounded_buffer request_worker_buffer(size_of_bounded_buffer);
vector<Bounded_buffer> worker_statistics_buffers(size_of_bounded_buffer, Bounded_buffer(size_of_bounded_buffer));
Semaphore worker_statistic_mutex(1);

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void *request_thread_routine(void* data_input);
void *statistics_thread_routine(void* data_input);
void *worker_channel_thread_routine(void* data_input);

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

  /* parsing arguments */
  int arguments;
  while ((arguments = getopt(argc, argv, "htnbw")) != -1 ) {
    switch(arguments) {
      case 'h':
        cout << "This client initializes a server, runs some requests via a client, and prints the output.\n"
             << "You can time it by inserting the '-t' flag." << endl
             << "You can set the number of data requests with the '-n' flag (default is 10)" << endl
             << "You can set the size of the bounded buffer with the '-b' flag (default is 1)" << endl
             << "You can set the number of worker threads with the '-w' flag (default is 1)" << endl;
        return 0;
      case 't':
        timer = true;
        break;
      case 'n':
        number_of_data_requests = strtol(optarg, NULL, 10);
        break;
      case 'b':
        size_of_bounded_buffer = strtol(optarg, NULL, 10);
        break;
      case 'w':
        number_of_worker_threads = strtol(optarg, NULL, 10);
        break;
      case '?':
        cerr << "Error: unknown flag(s)" << endl;
        return -1;
    }
  }


  //constants
  int number_of_data_requests_left[NUMBER_OF_REQUEST_AND_STATISTICS_THREADS] = {number_of_data_requests, number_of_data_requests, number_of_data_requests}; 
  int histogram_size[NUMBER_OF_REQUEST_AND_STATISTICS_THREADS] = {0, 0, 0};


  /* setting up server and client processes */
  pid_t client_process = fork();
  if (client_process >= 0) {
    if (client_process == 0) {
      cout << "CLIENT STARTED:" << endl;

      /* initial bounded buffer and mutexes */
    

      /* setup to create joinable threads */
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
      void *status;

      /* setting up the initial request and statistics threads.
       */
      pthread_t request_threads[NUMBER_OF_REQUEST_AND_STATISTICS_THREADS];
      pthread_t statistics_threads[NUMBER_OF_REQUEST_AND_STATISTICS_THREADS];

      int people_ids[NUMBER_OF_REQUEST_AND_STATISTICS_THREADS];

      int rc, i;
      for (i=0; i<NUMBER_OF_REQUEST_AND_STATISTICS_THREADS; i++) {
        people_ids[i] = i;
        rc = pthread_create(&request_threads[i], &attr, request_thread_routine, (void *) &people_ids[i]);
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
        }

        pthread_create(&statistics_threads[i], &attr, statistics_thread_routine, (void *) &people_ids[i]);
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
        }
      }

      /* setting up the worker threads */
      pthread_t worker_channel_threads[number_of_worker_threads];
      for (i=0; i<number_of_worker_threads; i++) {
        rc = pthread_create(&worker_channel_threads[i], &attr, worker_channel_thread_routine, NULL);
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
        }
      }

      // cout << "Establishing control channel... " << flush;
      // RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
      // cout << "done." << endl;;

      /* -- Start sending a sequence of requests */

      // string reply1 = chan.send_request("hello");
      // cout << "Reply to request 'hello' is '" << reply1 << "'" << endl;

      // string reply2 = chan.send_request("data Joe Smith");
      // cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << endl;

      // string reply3 = chan.send_request("data Jane Smith");
      // cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << endl;

      // string reply5 = chan.send_request("newthread");
      // cout << "Reply to request 'newthread' is " << reply5 << "'" << endl;
      // RequestChannel chan2(reply5, RequestChannel::CLIENT_SIDE);

      // string reply6 = chan2.send_request("data John Doe");
      // cout << "Reply to request 'data John Doe' is '" << reply6 << "'" << endl;

      // string reply7 = chan2.send_request("quit");
      // cout << "Reply to request 'quit' is '" << reply7 << "'" << endl;

      // string reply4 = chan.send_request("quit");
      // cout << "Reply to request 'quit' is '" << reply4 << "'" << endl;

      /* joining all threads, cleaning up */
      pthread_attr_destroy(&attr);
      for (i=0; i<NUMBER_OF_REQUEST_AND_STATISTICS_THREADS; i++) {
        rc = pthread_join(request_threads[i], &status);
        cout<<"got1"<<endl;
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_join() failed with error flag %d.\n", rc);
        }
        else {
          fprintf(stdout, "In simpleclient.C, pthread_join(), request thread [%d] finished with status  %ld.\n", i, (long)status);
        }

        rc = pthread_join(statistics_threads[i], &status);
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_join() failed with error flag %d.\n", rc);
        }
        else {
          fprintf(stdout, "In simpleclient.C, pthread_join(), statistics thread [%d] finished with status  %ld.\n", i, (long)status);
        }
      }

      for (i=0; i<number_of_worker_threads; i++) {
        rc = pthread_join(worker_channel_threads[i], &status);
        if (rc) {
          fprintf(stderr, "Error: In simpleclient.C, pthread_join() failed with error flag %d.\n", rc);
        }
        else {
          fprintf(stdout, "In simpleclient.C, pthread_join(), worker thread [%d] finished with status  %ld.\n", i, (long)status);
        }
      }

      usleep(100000);
      pthread_exit(NULL);

    }
    else {
      cout << "SERVER STARTED: " << endl;
      char* args_server[] = {"./dataserver", NULL};
      execv("./dataserver", args_server);
      /* it should only reach here if there is an error */
      cerr << "\nError: can't start server" << endl;
      return -3;
    }
  }
  else {
    cerr << "\nError: Cannot set up new processes in fork()..."<<endl;
    return -2;
  }
}

/*--------------------------------------------------------------------------*/
/* THREAD ROUTINES */
/*--------------------------------------------------------------------------*/
void *request_thread_routine(void* data_input) {
  /* The request thread generates a bunch of requests for the 3 'people' for the server.
   * It puts the requests in a bounded buffer.
   */
  int person = *(int*) data_input;
  if (person == 0) {
    while(data_requests_produced_size_joe< number_of_data_requests) {
      data_requests_produced_size_joe += 1;
      request_worker_buffer.produce(person);
    }
  }

  else if (person == 1) {
    while(data_requests_produced_size_jane < number_of_data_requests) {
      data_requests_produced_size_jane += 1;
      request_worker_buffer.produce(person);
    }
  }


  else {
    while(data_requests_produced_size_john < number_of_data_requests) {
      data_requests_produced_size_john += 1;
      request_worker_buffer.produce(person);
    }
  }

  printf("Finished request thread routine.\n");
  pthread_exit(NULL);
}

void *statistics_thread_routine(void* data_input) {
  /* The statistics thread pulls the data from another bounded buffer with [int name, int server_response]
   * and adds it to the bounded buffers
   */
  int person = *(int*) data_input;
  while(worker_statistics_buffers[person].size() > 0 || histogram_size[person] < number_of_data_requests) {
    int result = worker_statistics_buffers[person].consume();
    if (result >= server_responses[person].size()) {
      server_responses[person].resize(result+1);
    }
    server_responses[person][result] += 1;
  }
  pthread_exit(NULL);
}

void *worker_channel_thread_routine(void* data) {
  /* the worker thread pulls requests from the bounded buffer, sends them to the server, and returns the result 
   * to the other bounded buffer, with the name type 
   */

  while(request_worker_buffer.size() > 0 || data_requests_consumed_size < 3*number_of_data_requests) {
    RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
    int person = request_worker_buffer.consume();
    string reply = chan.send_request(strcat("data ", people[person]));
    int result = atoi(reply.c_str());

    cout<<"in worker, result is "<<result<<endl<<flush;
    worker_statistics_buffers[person].produce(result);
  }
  pthread_exit(NULL);
}

