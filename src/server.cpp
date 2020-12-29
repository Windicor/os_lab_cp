#include "server.h"

#include <pthread.h>
#include <signal.h>

#include <iostream>

#include "m_zmq.h"

using namespace std;

Server::Server() {
  log("Starting server...");
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);
  endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  subscriber_ = make_unique<Socket>(context_, SocketType::SUBSCRIBER, endpoint);
}

Server::~Server() {
  log("Destroying server...");
  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);
  } catch (exception& ex) {
    log("Server wasn't destroyed: "s + ex.what());
  }
}

void Server::send(const Message& message) {
  publiser_->send(message);
  log("Message sended from server: "s + message.get_stats());
}

Message Server::receive() {
  Message message = subscriber_->receive();
  log("Message received by server: "s + message.get_stats());
  return message;
}

void Server::log(std::string message) {
  logger_.log(move(message));
}