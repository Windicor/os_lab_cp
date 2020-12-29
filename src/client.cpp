#include "client.h"

#include <iostream>

#include "m_zmq.h"

using namespace std;

const int ERR_LOOP = 2;

void* second_thread(void* cli_arg) {
  Client* client_ptr = (Client*)cli_arg;
  try {
    string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
    client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, endpoint);

    while (true) {
      Message msg = client_ptr->receive();
      if (msg.command == CommandType::ERROR) {
        if (client_ptr->terminated_) {
          return NULL;
        } else {
          throw runtime_error("Can't receive message");
        }
      }
      client_ptr->log("Message received by client: "s + msg.get_stats());
      //cout << msg.text << endl;
    }

  } catch (exception& ex) {
    client_ptr->log("Client exctption: "s + ex.what() + "\nTerminated by exception on client receive loop"s);
    exit(ERR_LOOP);
  }
  return NULL;
}

Client::Client() {
  log("Starting client...");
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);

  if (pthread_create(&second_thread_id, 0, second_thread, this) != 0) {
    throw runtime_error("Can't run second thread");
  }
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

void Client::send(const Message& message) {
  publiser_->send(message);
  log("Message sended from client: "s + message.get_stats());
}

Message Client::receive() {
  return subscriber_->receive();
}

void Client::log(string message) {
  logger_.log(move(message));
}