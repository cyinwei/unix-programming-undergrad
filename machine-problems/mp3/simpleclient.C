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

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <stdlib.h>

#include <sys/time.h>

#include <errno.h>
#include <unistd.h>

#include "reqchannel.H"
#include "semaphore.H"
#include "bounded_buffer.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* CONSTANTS, INITIAL VALUES */
/*--------------------------------------------------------------------------*/

/* these are the default values */
const int RT_ST_SIZE = 3;
int WT_SIZE = 10;
int BB_SIZE = 3;
int REQUEST_SIZE = 10;
bool timer = false;

/* used for name lookups from thread routines */
const string names[3] = {"Joe Smith", "Jane Smith", "John Doe"};

/* used for timing */
struct timeval start_time;
struct timeval end_time;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

vector<int> Joe_histogram(100, 0);
vector<int> Jane_histogram(100, 0);
vector<int> John_histogram(100, 0);

RequestChannel* chan;

/*--------------------------------------------------------------------------*/
/* THREAD OBJECTS & DATA */
/*--------------------------------------------------------------------------*/

/* mutexes to create new threads */
Semaphore newthread_mutex(1);

/* Buffer for communication between the request threads and the worker threads */
BoundedBuffer buffer(BB_SIZE);

/* Used for communication between the worker and statistics threads */
queue<int> Joe_replies;
queue<int> Jane_replies;
queue<int> John_replies;
Semaphore Joe_replies_mutex(1);
Semaphore Jane_replies_mutex(1);
Semaphore John_replies_mutex(1);




/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS : SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

string print_histograms() {
  stringstream result;
  result << "\nHere are the histograms.\n"
         << "This is Joe's histogram\n";
  for (int i=0; i<Joe_histogram.size(); i++) {
    result << "[" << int2string(i) << "] ==> [" << int2string(Joe_histogram[i]) << "]\n";
  }
  result << "\nThis is Jane's histogram\n";
  for (int i=0; i<Jane_histogram.size(); i++) {
    result << "[" << int2string(i) << "] ==> [" << int2string(Jane_histogram[i]) << "]\n";
  }
  result << "\nThis is John's histogram\n";
  for (int i=0; i<John_histogram.size(); i++) {
    result << "[" << int2string(i) << "] ==> [" << int2string(John_histogram[i]) << "]\n";
  }
  return result.str();
}

/* timing function from MP1 */
long time_diff(struct timeval * tp1, struct timeval * tp2) {
  /* Prints to stdout the difference, in seconds and museconds, between two
     timevals. */
  long sec = tp2->tv_sec - tp1->tv_sec;
  long musec = tp2->tv_usec - tp1->tv_usec;
  if (musec < 0) {
    musec += 1000000;
    sec--;
  }
return musec;
}

/*--------------------------------------------------------------------------*/
/* THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

void* request_routine(void* person) {
  //cout<<"In request routine\n\n"<<flush;
  int nameid = * (int *) person;
  string name = names[nameid];

  /* critical section, don't want multiple newthread requests at the same time,
   * or we get undefined behavior.
   */
  newthread_mutex.P();
  //cout<<"In critical section" << endl << flush;
  string newchan = chan->send_request("newthread");
  RequestChannel ochan(newchan, RequestChannel::CLIENT_SIDE);
  //cout<<"new request channel "<<newchan<<endl<<flush;
  newthread_mutex.V();
  /* end of critical section */

  cout<<"done"<<endl<<flush;

  string reply_str;
  int reply;
  string request = "data " + name;

  for (int i=0; i<REQUEST_SIZE; i++) {
    cout<<"in for loop"<<endl<<flush;
    reply_str = ochan.send_request(request);
    reply = atoi(reply_str.c_str());
    cout<<"reply is "<<reply<<endl<<flush;
    buffer.produce(nameid, reply);
  }
  ochan.send_request("quit");
  pthread_exit(NULL);
}

void* worker_routine(void* no_input) {
  vector<int> reply;
  while (true) {
    reply = buffer.consume();
    if (reply[0] == 0) {
      Joe_replies_mutex.P();
      Joe_replies.push(reply[1]);
      Joe_replies_mutex.V();
    }
    else if (reply[0] == 1) {
      Jane_replies_mutex.P();
      Jane_replies.push(reply[1]);
      Jane_replies_mutex.V();
    }
    else if (reply[0] == 2) {
      John_replies_mutex.P();
      John_replies.push(reply[1]);
      John_replies_mutex.V();
    }
    else {
      cerr<<"Error: In simpleclient.C's worker_routine(), bounded buffer id is invalid.\n";
    }
  }/* to be closed by parent thread */
}

void* statistic_routine(void* person) {
  int count = 0;
  int reply;
  const int nameid = *(int *) person;
  while (count < REQUEST_SIZE) {
    if (nameid == 0) {
      Joe_replies_mutex.P();
      if (!Joe_replies.empty()) {
        reply = Joe_replies.front();
        Joe_replies.pop();
        Joe_histogram[reply] += 1;
        count++; 
      }
      Joe_replies_mutex.V();
    }
    else if (nameid == 1) {
      Jane_replies_mutex.P();
      if (!Jane_replies.empty()) {
        reply = Jane_replies.front();
        Jane_replies.pop();
        Jane_histogram[reply] += 1;
        count++;
      }
      Jane_replies_mutex.V();
    }
    else if (nameid == 2) {
      John_replies_mutex.P();
      if (!John_replies.empty()) {
        reply = John_replies.front();
        John_replies.pop();
        John_histogram[reply] += 1;
        count++;
      }
      John_replies_mutex.V();
    }
    else {
      cerr << "Error in simpleclient.C::statistic_routine(): invalid nameid input.\n";
      pthread_exit(NULL);
    }
  }
  pthread_exit(NULL);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

  /* getting input arguments */
  int arguments;
  while ((arguments = getopt(argc, argv, "htn:b:w:")) != -1 ) {
    switch(arguments) {
      case 'h':
        cout << "This client initializes a server, runs some requests via a client, and prints the output.\n"
             << "You can time it by inserting the '-t' flag." << endl
             << "You can set the number of data requests with the '-n' flag (default is 10)" << endl
             << "You can set the size of the bounded buffer with the '-b' flag (default is 3)" << endl
             << "You can set the number of worker threads with the '-w' flag (default is 10)" << endl;
        return 0;
      case 't':
        timer = true;
        break;
      case 'n':
        REQUEST_SIZE = atoi(optarg);
        break;
      case 'b':
        BB_SIZE = atoi(optarg);
        break;
      case 'w':
        WT_SIZE = atoi(optarg);
        break;
      case '?':
        cout << "Error: unknown flag(s), type -h for help\n" << endl;
        return -1;
    }
  }


  /* creating the client & server processes */
  pid_t client_process = fork();
  if (client_process < 0) {
    cerr<<"Error in simpleclient.C: cannot fork process.  Bye.\n";
    return -1;
  }
  else if (client_process == 0) {
    /* wait for server to start */
    usleep(10000);
    cout << "CLIENT STARTED:" << endl;

    /* if we want to time our client process. */
    if (timer) {
      if (gettimeofday(&start_time, 0) != 0) {
        cerr << "Error: In simpleclient.C, timeval start_time cannot start.\n";
      }
    }

    /* setting initial channel */
    cout << "Establishing control channel... " << flush;
    chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
    cout << "done." << endl;

    /* setup to create joinable threads */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    void *status;
    int rc, i;

    /* initialized stable memory to passs name info into pthread_create() */
    int nameids[RT_ST_SIZE];

    /* intializing request threads */
    pthread_t request_threads[RT_ST_SIZE];
    for (i=0; i<RT_ST_SIZE; i++) {
      nameids[i] = i;
      rc = pthread_create(&request_threads[i], &attr, request_routine, (void *) &nameids[i]);
      if (rc) {
        fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
      }
    }
    
    /*initializing worker threads */
    pthread_t worker_threads[WT_SIZE];
    for (i=0; i<WT_SIZE; i++) {
      rc = pthread_create(&worker_threads[i], &attr, worker_routine, NULL);
      if (rc) {
        fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
      }
    }
    
    /*initializing statistics threads */
    pthread_t statistic_threads[RT_ST_SIZE];
    for (i=0; i<RT_ST_SIZE; i++) {
      nameids[i] = i;
      rc = pthread_create(&statistic_threads[i], &attr, statistic_routine, &nameids[i]);
      if (rc) {
        fprintf(stderr, "Error: In simpleclient.C, pthread_create() failed with error flag %d.\n", rc);
      }
    } 

    /* freeing the attr object */
    pthread_attr_destroy(&attr);

    for (i=0; i<RT_ST_SIZE; i++) {
      rc = pthread_join(request_threads[i], &status);
      cout<<"got1"<<endl;
      if (rc) {
        fprintf(stderr, "Error: In simpleclient.C, pthread_join() failed with error flag %d.\n", rc);
      }
    }

    chan->send_request("quit");
    delete chan;
    usleep(1000000);

    for (i=0; i<RT_ST_SIZE; i++) {
      rc = pthread_join(statistic_threads[i], &status);
    }

    /* statistics are done, now free the workers */
    for (i=0; i<WT_SIZE; i++) {
      pthread_cancel(worker_threads[i]);
    }

    string result = print_histograms();
    cout<<result;

    if (timer) {
      if (gettimeofday(&end_time, 0) != 0) {
        cerr<<"Error: In simpleclient.C, timeval end_time failed to get time.\n";
      }
      else {
        cout <<"\n\n\n"
             <<"Total elapsed time is: "<<time_diff(&start_time, &end_time)<<" microseconds."<<endl;
      }
    }

    pthread_exit(NULL);
  }
  else {
    cout << "SERVER STARTED: " << endl;
    char* args_server[] = {"./dataserver", NULL};
    execv("./dataserver", args_server);
    /* it should only reach here if there is an error */
    cerr << "\nError: can't start server" << endl;
    return -2;
  }


  
}