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
  server_ptr->log("Server is terminated by user");
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
    server.log("Server is started correctly");

    while (true) {
      Message msg = server.receive();
      if (msg.command == CommandType::PING) {
        server.send(Message::ping_message());
      }
    }
  } catch (exception& ex) {
    cout << ("Server exception: "s + ex.what() + "\nServer is terminated by exception");
  }
  return ERR_TERMINATED;
}
