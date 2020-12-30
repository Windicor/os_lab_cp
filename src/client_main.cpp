#include <signal.h>

#include <iostream>
#include <string>

#include "client.h"

using namespace std;

const int ERR_TERMINATED = 1;
const int UNIVERSAL_MESSAGE_ID = -256;

Client* client_ptr = nullptr;
void TerminateByUser(int) {
  if (client_ptr != nullptr) {
    client_ptr->~Client();
    client_ptr->log("Client is terminated by user");
  }
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

    Client client;
    client_ptr = &client;
    client.log("Client is started correctly");

    client.connect_to_server();

    string text;
    while (getline(cin, text)) {
      client.send_text_msg(move(text));
    }

  } catch (exception& ex) {
    cout << (to_string(getpid()) + " Client exception: "s + ex.what() + "\nClient terminated by exception"s) << endl;
    exit(ERR_TERMINATED);
  }
  return 0;
}