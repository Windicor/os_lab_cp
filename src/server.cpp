#include "server.h"

#include <pthread.h>
#include <signal.h>

#include <filesystem>
#include <iostream>
#include <sstream>

#include "m_zmq.h"

using namespace std;

void Online::add_user(std::string username, int id) {
  id_to_username_[id] = username;
  username_to_id_[move(username)] = id;
}
void Online::remove_user(int id) {
  auto it = id_to_username_.find(id);
  if (it != id_to_username_.end()) {
    string user = it->second;
    auto it2 = username_to_id_.find(move(user));
    id_to_username_.erase(it);
    username_to_id_.erase(it2);
  } else {
    cerr << "User to remove not found" << endl;
  }
}
optional<int> Online::get_id(std::string username) const {
  auto it = username_to_id_.find(move(username));
  if (it != username_to_id_.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}
optional<std::string> Online::get_username(int id) const {
  auto it = id_to_username_.find(id);
  if (it != id_to_username_.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}
bool Online::check_username(std::string username) const {
  auto it = username_to_id_.find(move(username));
  return it != username_to_id_.end();
}

bool Rooms::add_room(int id1, int id2) {
  if (rooms_.count(id1) > 0 || rooms_.count(id2) > 0) {
    return false;
  }
  rooms_[id1] = id2;
  rooms_[id2] = id1;
  return true;
}
void Rooms::remove_room(int id) {
  int id2 = rooms_[id];
  rooms_.erase(id);
  rooms_.erase(id2);
}
optional<int> Rooms::get_companion(int id) const {
  auto it = rooms_.find(move(id));
  if (it != rooms_.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}
bool Rooms::check_companion(int id) const {
  return (bool)get_companion(id);
}

Server::Server() {
  log("Starting server...");
  context_ = create_zmq_context();

  if (!filesystem::exists(ENDPOINT_FOLDER)) {
    filesystem::create_directory(ENDPOINT_FOLDER);
  }

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

void Server::send_from_user_to_user(int from_id, std::shared_ptr<Message> message) {
  auto opt = rooms_.get_companion(from_id);
  if (!opt) {
    send_to_user(from_id, Message::error_message());
    return;
  }
  int to_id = *opt;
  send_to_user(to_id, message);
}

shared_ptr<Message> Server::receive() {
  shared_ptr<Message> message = subscriber_->receive();
  log("Message received by server: "s + message->get_stats());
  if (message->type() == MessageType::TEXT) {
    log("Text: \""s + string(((TextMessage*)message.get())->text) + "\""s);
  }
  if (message->type() == MessageType::FILE) {
    log("Package size: "s + to_string(((FileMessage*)message.get())->size));
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

void print_bool(bool b) {
  cout << b << endl;
}

void Server::create_room(int from_id, string username) {
  auto opt = online_.get_id(username);
  cout << *online_.get_username(from_id) << "a a" << username << "a" << endl;
  if (!opt || rooms_.check_companion(from_id) || rooms_.check_companion(*opt) || *online_.get_username(from_id) == username) {
    send_to_user(from_id, make_shared<Message>(CommandType::CREATE_CHAT, 0, from_id, 0));
    return;
  }
  int to_id = *opt;
  rooms_.add_room(from_id, to_id);
  send_to_user(from_id, make_shared<TextMessage>(CommandType::CREATE_CHAT, 0, from_id, *online_.get_username(to_id), 1));
  send_to_user(to_id, make_shared<TextMessage>(CommandType::CREATE_CHAT, 0, to_id, *online_.get_username(from_id), 1));
}

void Server::exit_room(int id) {
  auto opt = rooms_.get_companion(id);
  if (opt) {
    int companion_id = *opt;
    send_to_user(companion_id, make_shared<Message>(CommandType::LEFT_CHAT, 0, companion_id, 0));
  }
  rooms_.remove_room(id);
}

void Server::remove_connection(int id) {
  exit_room(id);
  online_.remove_user(id);
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
    online_.add_user(uname, msg_ptr->from_id);
  } else {
    send_to_user(msg_ptr->from_id, make_shared<Message>(CommandType::REGISTER, 0, msg_ptr->from_id, 0));
  }
}

void Server::login_form(std::shared_ptr<Message> msg_ptr) {
  TextMessage* text_msg_ptr = (TextMessage*)msg_ptr.get();
  istringstream iss(text_msg_ptr->text);
  LogAndPas lap;
  iss >> lap;

  auto opt_uname = security.get_username(move(lap));
  if (opt_uname && !online_.check_username(*opt_uname)) {
    send_to_user(msg_ptr->from_id, make_shared<Message>(CommandType::LOGIN, 0, msg_ptr->from_id, 1));
    online_.add_user(*opt_uname, msg_ptr->from_id);
  } else {
    send_to_user(msg_ptr->from_id, make_shared<Message>(CommandType::LOGIN, 0, msg_ptr->from_id, 0));
  }
}
