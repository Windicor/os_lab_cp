#include <signal.h>

#include <iostream>
#include <string>

#include "server.h"

using namespace std;

const int ERR_TERMINATED = 1;

Server* server_ptr = nullptr;
void TerminateByUser(int) {
  if (server_ptr != nullptr) {
    server_ptr->~Server();
  }
  cerr << to_string(getpid()) + " Server is terminated by user"s << endl;
  exit(0);
}

int main() {
  try {
    if (signal(SIGINT, TerminateByUser) == SIG_ERR) {
      throw runtime_error("Can't set SIGINT signal");
    }
    if (signal(SIGSEGV, TerminateByUser) == SIG_ERR) {
      throw runtime_error("Can't set SIGSEGV signal");
    }

    Server server;
    server_ptr = &server;
    cerr << to_string(getpid()) + " Server is started correctly"s << endl;

    while (true) {
      Message msg = server.receive();
      cout << "Message on server" << msg.text << endl;
    }
  } catch (exception& ex) {
    cerr << to_string(getpid()) + " Server exception: "s << ex.what() << endl;
  }
  cerr << to_string(getpid()) + " Server is terminated by exception"s << endl;
  return ERR_TERMINATED;
}
