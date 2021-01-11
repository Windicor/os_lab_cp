#include <signal.h>

#include <iostream>
#include <sstream>
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

const string EXIT_COMMAND = "\\exit";
const string FILE_COMMAND = "\\file";

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
    client.enter_in_system();

    cout << "Enter your companion username" << endl;
    string text;
    while (getline(cin, text)) {
      if (text == "") {
        continue;
      }
      if (text.size() > TextMessage::MAX_MESSAGE_SIZE) {
        cout << "Too long message" << endl;
        continue;
      }

      if (client.status == Client::Status::IN_CHAT) {
        if (text == EXIT_COMMAND) {
          client.left_chat();
        } else if (text.size() > FILE_COMMAND.size() + 1 && text.substr(0, FILE_COMMAND.size()) == FILE_COMMAND && text[FILE_COMMAND.size()] == ' ') {
          client.send_file_msg(text.substr(FILE_COMMAND.size() + 1));
        } else {
          client.send_text_msg(move(text));
        }
      } else {
        client.enter_chat(move(text));
      }
    }

  } catch (exception& ex) {
    cout << (to_string(getpid()) + " Client exception: "s + ex.what() + "\nClient terminated by exception"s) << endl;
    exit(ERR_TERMINATED);
  }
  return 0;
}