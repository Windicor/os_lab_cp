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
  }
  cerr << to_string(getpid()) + " Terminated by user"s << endl;
  exit(0);
}

int main(int argc, char const* argv[]) {
  if (argc != 3) {
    cerr << argc;
    cerr << "USAGE: " << argv[0] << " <id> <parrent_pub_endpoint" << endl;
  }

  try {
    if (signal(SIGINT, TerminateByUser) == SIG_ERR) {
      throw runtime_error("Can't set SIGINT signal");
    }
    if (signal(SIGSEGV, TerminateByUser) == SIG_ERR) {
      throw runtime_error("Can't set SIGSEGV signal");
    }

    Client client;
    client_ptr = &client;
    cerr << to_string(getpid()) + " Client is started correctly"s << endl;

    string text;
    while (cin >> text) {
      Message msg(CommandType::RETURN, 0, 0, text);
      client.send(msg);
      cerr << "Message sended" << endl;
    }

  } catch (exception& ex) {
    cerr << to_string(getpid()) + " Client exception: "s << ex.what() << "\nTerminated by exception" << endl;
    exit(ERR_TERMINATED);
  }
  cerr << to_string(getpid()) + " Client is finished correctly"s << endl;
  return 0;
}