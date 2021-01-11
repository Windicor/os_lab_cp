#include <signal.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "server.h"

using namespace std;

const int ERR_TERMINATED = 1;

Server* server_ptr = nullptr;
void TerminateByUser(int) {
  if (server_ptr != nullptr) {
    server_ptr->~Server();
    server_ptr->log("Server is terminated by user");
  }
  exit(0);
}

void parse_cmd(Server& server, shared_ptr<Message> msg_ptr) {
  switch (msg_ptr->command) {
    case CommandType::CONNECT:
      server.add_connection(msg_ptr->from_id);
      break;
    case CommandType::DISCONNECT:
      server.remove_connection(msg_ptr->from_id);
      break;
    case CommandType::TEXT:
      if (msg_ptr->type() != MessageType::TEXT) {
        server.log("Text command in non text message");
        break;
      }
      server.send_from_user_to_user(msg_ptr->from_id, msg_ptr);
      break;
    case CommandType::REGISTER:
      if (msg_ptr->type() != MessageType::TEXT) {
        server.log("Register command in non text message");
        break;
      }
      server.register_form(msg_ptr);
      break;
    case CommandType::LOGIN:
      if (msg_ptr->type() != MessageType::TEXT) {
        server.log("Login command in non text message");
        break;
      }
      server.login_form(msg_ptr);
      break;
    case CommandType::CREATE_CHAT:
      if (msg_ptr->type() != MessageType::TEXT) {
        server.log("Register command in non text message");
        break;
      }
      server.create_room(msg_ptr->from_id, ((TextMessage*)msg_ptr.get())->text);
      break;
    case CommandType::LEFT_CHAT:
      server.exit_room(msg_ptr->from_id);
      break;
    case CommandType::FILE_NAME:
      if (msg_ptr->type() != MessageType::TEXT) {
        server.log("FileName command in non text message");
        break;
      }
      server.send_from_user_to_user(msg_ptr->from_id, msg_ptr);
      break;
    default:
      throw logic_error("Unimplemented command type");
  }
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
      shared_ptr<Message> msg_ptr = server.receive();
      parse_cmd(server, msg_ptr);
    }
  } catch (exception& ex) {
    cerr << ("Server exception: "s + ex.what() + "\nServer is terminated by exception");
  }
  return ERR_TERMINATED;
}
