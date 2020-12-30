#include "server.h"

#include <pthread.h>
#include <signal.h>

#include <iostream>
#include <sstream>

#include "m_zmq.h"

using namespace std;

Server::Server() {
  log("Starting server...");
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
  general_publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, move(endpoint));
  endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  subscriber_ = make_unique<Socket>(context_, SocketType::SUBSCRIBER, move(endpoint));
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

void Server::send_to_general(shared_ptr<Message> message) {
  general_publiser_->send(message);
  log("Message sended from server: "s + message->get_stats());
}

void Server::send_to_user(int id, std::shared_ptr<Message> message) {
  auto it = id_to_publisher_.find(id);
  if (it == id_to_publisher_.end()) {
    log("Message to id, that does not exist");
    return;
  }
  message->from_id = 0;
  message->to_id = id;
  id_to_publisher_[id]->send(message);
  log("Message sended from server to "s + to_string(id) + ": "s + message->get_stats());
}
/*
void Server::send_from_user_to_user(int from_id, int to_id, std::shared_ptr<Message> message) {
  auto it = id_to_publisher_.find(from_id);
  if (it == id_to_publisher_.end()) {
    log("Message to id, that does not exist");
    return;
  }
  message->from_id = from_id;
  message->to_id = to_id;
  id_to_publisher_[to_id]->send(message);
  log("Message sended from " + to_string(from_id) + " to "s + to_string(to_id) + ": "s + message->get_stats());
}
*/

shared_ptr<Message> Server::receive() {
  shared_ptr<Message> message = subscriber_->receive();
  log("Message received by server: "s + message->get_stats());
  if (message->type() == MessageType::TEXT) {
    log("Text: \""s + string(((TextMessage*)message.get())->text) + "\""s);
  }
  return message;
}

void Server::log(std::string message) {
  logger_.log(move(message));
}

void Server::add_connection(int id) {
  int new_id = ++id_cntr;
  string endpoint = create_endpoint(EndpointType::SERVER_PUB, new_id);
  id_to_publisher_[new_id] = make_unique<Socket>(context_, SocketType::PUBLISHER, move(endpoint));
  endpoint = create_endpoint(EndpointType::CLIENT_PUB, new_id);
  subscriber_->subscribe(move(endpoint));

  log("Connection added");
  send_to_general(make_shared<Message>(CommandType::CONNECT, 0, id, new_id));
}

void Server::remove_connection(int id) {
  id_to_publisher_.erase(id);
  string endpoint = create_endpoint(EndpointType::CLIENT_PUB, id);
  subscriber_->unsubscribe(move(endpoint));
  log("Connection removed");
}

void Server::register_form(std::shared_ptr<Message> msg_ptr) {
  TextMessage* text_msg_ptr = (TextMessage*)msg_ptr.get();
  istringstream iss(text_msg_ptr->text);
  string uname;
  LogAndPas lap;
  iss >> uname >> lap;
  if (security.Register(uname, move(lap))) {
    send_to_user(msg_ptr->from_id, make_shared<Message>(CommandType::REGISTER, 0, msg_ptr->from_id, 1));
  } else {
    send_to_user(msg_ptr->from_id, make_shared<Message>(CommandType::REGISTER, 0, msg_ptr->from_id, 0));
  }
  id_to_username_[msg_ptr->from_id] = uname;
}