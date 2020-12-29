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
  general_publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);
  endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  subscriber_ = make_unique<Socket>(context_, SocketType::SUBSCRIBER, endpoint);
}

Server::~Server() {
  log("Destroying server...");
  try {
    general_publiser_ = nullptr;
    subscriber_ = nullptr;
    for (auto& [_, ptr] : id_to_publisher_) {
      ptr = nullptr;
    }
    destroy_zmq_context(context_);
  } catch (exception& ex) {
    log("Server wasn't destroyed: "s + ex.what());
  }
}

void Server::send(const Message& message) {
  general_publiser_->send(message);
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

void Server::add_connection(int id) {
  int new_id = id_cntr++;
  string endpoint = create_endpoint(EndpointType::SERVER_PUB, new_id);
  id_to_publisher_[new_id] = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);
  endpoint = create_endpoint(EndpointType::CLIENT_PUB, new_id);
  subscriber_->subscribe(endpoint);

  send(Message(CommandType::CONNECT, 0, id, new_id));
}
