#include "server.h"

#include <pthread.h>
#include <signal.h>

#include <iostream>

#include "m_zmq.h"

using namespace std;

Server::Server() {
  cerr << to_string(getpid()) + " Starting server..."s << endl;
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);
  endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  subscriber_ = make_unique<Socket>(context_, SocketType::SUBSCRIBER, endpoint);
}

Server::~Server() {
  cerr << to_string(getpid()) + " Destroying server..."s << endl;
  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);
  } catch (exception& ex) {
    cerr << "Server wasn't destroyed: " << ex.what() << endl;
  }
}

void Server::send(Message message) {
  publiser_->send(message);
}

Message Server::receive() {
  return subscriber_->receive();
}