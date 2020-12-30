#include "client.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

#include "m_zmq.h"

using namespace std;

const int ERR_LOOP = 2;
const int MESSAGE_WAITING_TIME = 1;

void* second_thread(void* cli_arg) {
  Client* client_ptr = (Client*)cli_arg;
  try {
    string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
    client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, endpoint);

    while (true) {
      shared_ptr<Message> msg_ptr = client_ptr->receive();
      if (msg_ptr->command == CommandType::ERROR) {
        if (client_ptr->terminated_) {
          return NULL;
        } else {
          throw runtime_error("Can't receive message");
        }
      }
      if (msg_ptr->to_id != client_ptr->id()) {
        continue;
      }
      client_ptr->log("Message received by client: "s + msg_ptr->get_stats());
      switch (msg_ptr->command) {
        case CommandType::CONNECT:
          client_ptr->id_ = msg_ptr->value;
          client_ptr->server_is_avaible_ = true;

          endpoint = create_endpoint(EndpointType::CLIENT_PUB, client_ptr->id());
          client_ptr->publiser_ = nullptr;
          client_ptr->publiser_ = make_unique<Socket>(client_ptr->context_, SocketType::PUBLISHER, endpoint);

          endpoint = create_endpoint(EndpointType::SERVER_PUB, client_ptr->id());
          client_ptr->subscriber_ = nullptr;
          client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, endpoint);
          break;

        default:
          throw logic_error("Undefined command type");
          break;
      }
    }

  } catch (exception& ex) {
    client_ptr->log("Client exctption: "s + ex.what() + "\nTerminated by exception on client receive loop"s);
    exit(ERR_LOOP);
  }
  return NULL;
}

void Client::connect_to_server() {
  while (!server_is_avaible_) {
    cout << "Trying to connect to the server..." << endl;
    send(Message::connect_message(id_));
    sleep(MESSAGE_WAITING_TIME);
  }
  cout << "Connected to server" << endl;
}

Client::Client() {
  log("Starting client...");
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);

  if (pthread_create(&second_thread_id_, 0, second_thread, this) != 0) {
    cout << "Can't run second thread" << endl;
    exit(ERR_LOOP);
  }

  srand(time(NULL) + clock());
  id_ = rand();
}

Client::~Client() {
  if (terminated_) {
    log("Client double termination");
    return;
  }

  log("Destroying client...");
  terminated_ = true;

  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);
  } catch (exception& ex) {
    log("Client wasn't destroyed: "s + ex.what());
  }
}

int Client::id() const {
  return id_;
}

void Client::send(shared_ptr<Message> message) {
  publiser_->send(message);
  log("Message sended from client: "s + message->get_stats());
}

shared_ptr<Message> Client::receive() {
  return subscriber_->receive();
}

void Client::log(string message) {
  logger_.log(move(message));
}

void Client::send_text_msg(string message) {
  send(make_shared<TextMessage>(CommandType::TEXT, id_, 0, move(message)));
}