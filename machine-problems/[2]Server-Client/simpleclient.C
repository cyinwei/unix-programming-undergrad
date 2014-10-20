/* 
    File: simpleclient.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

    Simple client main program for MP2 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include <ctime>

#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include "reqchannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

string helloReply(string input) {
  if (input == "hello") 
    return "hello to you too";
  else return "";
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

  /* getting the input operations */
  int c;
  bool timer, invocation;

  std::clock_t timeWholeStart;
  std::clock_t timeHelloStart;
  std::clock_t timeHelloEnd;
  std::clock_t timeLocalHelloStart;

  int helloTime;
  int helloLocalTime;

  while ((c = getopt(argc, argv, "hti")) != -1 ) {
    switch(c) {
      case 't':
        timer = true;
        break;
      case 'i':
        invocation = true;
        break;
      case 'h':
        cout << "This client initializes a server, runs some requests via a client, and prints the output.\n"
             << "You can time it by inserting the '-t' flag." << endl
             << "You can run a location timer vs a server timer with the '-i' flag" << endl;
        return 0;
      case '?':
        cerr << "Error: unknown flag(s)" << endl;
        return -2;
    }
  }

  pid_t clientProcess = fork();
  if (clientProcess >= 0) {
    if (clientProcess == 0) {
      cout << "CLIENT STARTED:" << endl;

      if (timer) {
        timeWholeStart = std::clock();
      }

      cout << "Establishing control channel... " << flush;
      RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
      cout << "done." << endl;

      /* -- Start sending a sequence of requests */

      if (timer || invocation)
        timeHelloStart = std::clock();

      string reply1 = chan.send_request("hello");

      if (timer || invocation)
        helloTime = std::clock() - timeHelloStart;

      cout << "Reply to request 'hello' is '" << reply1 << "'" << endl;

      if (invocation) {
        timeLocalHelloStart = std::clock();
        string notUsed = helloReply("hello");
        helloLocalTime = std::clock() - timeLocalHelloStart;
      }

      string reply2 = chan.send_request("data Joe Smith");
      cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << endl;

      string reply3 = chan.send_request("data Jane Smith");
      cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << endl;

      for(int i = 0; i < 100; i++) {
        string request_string("data TestPerson" + int2string(i));
        string reply_string = chan.send_request(request_string);
      cout << "reply to request " << i << ":" << reply_string << endl;;
      }
     
      string reply4 = chan.send_request("quit");
      cout << "Reply to request 'quit' is '" << reply4 << endl;

      if (timer) {
        cout << "Time for the whole process was: " << (std::clock()-timeWholeStart) << endl;
      }
      if (invocation || timer) {
        cout << "Time for the hello request was: " << helloTime << endl;
      }
      if (invocation) {
        cout << "Time for the local hello request was " << helloLocalTime << endl;
      }

      usleep(1000000);
      return 0;
    }
    else {
      cout << "SERVER STARTED: " << endl;
      char* args[] = {"./dataserver", NULL};
      execv("./dataserver", args);
      /* it should only reach here if there is an error */
      cerr << "\nError: can't start server" << endl;
      return -1;
    }
  }
  else {
    cerr <<"Error: can't create server process." << endl;
    return -3;
  }
}
