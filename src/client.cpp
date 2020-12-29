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
      cerr << "Message on client: " << static_cast<int>(msg.command) << msg.text << endl;
    }

  } catch (exception& ex) {
    cerr << "Client exctption: " << ex.what() << "\nTerminated by exception on client receive loop" << endl;
    exit(ERR_LOOP);
  }
  return NULL;
}

Client::Client() {
  cerr << to_string(getpid()) + " Starting client..."s << endl;
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);

  if (pthread_create(&second_thread_id, 0, second_thread, this) != 0) {
    throw runtime_error("Can't run second thread");
  }
}

Client::~Client() {
  if (terminated_) {
    cerr << to_string(getpid()) + " Client double termination"s << endl;
    return;
  }

  cerr << to_string(getpid()) + " Destroying client..."s << endl;
  terminated_ = true;

  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);
  } catch (exception& ex) {
    cerr << to_string(getpid()) + " " + " Client wasn't destroyed: "s << ex.what() << endl;
  }
}

void Client::send(const Message& message) {
  publiser_->send(message);
}

Message Client::receive() {
  return subscriber_->receive();
}
