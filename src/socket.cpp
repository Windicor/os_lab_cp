#include "socket.h"

#include <iostream>
#include <stdexcept>

using namespace std;

Socket::Socket(void* context, SocketType socket_type, std::string endpoint)
    : socket_type_(socket_type), endpoint_(endpoint) {
  socket_ = create_zmq_socket(context, socket_type_);
  switch (socket_type_) {
    case SocketType::PUBLISHER:
      bind_zmq_socket(socket_, endpoint);
      break;
    case SocketType::SUBSCRIBER:
      connect_zmq_socket(socket_, endpoint);
      break;
    default:
      throw logic_error("Undefined connection type");
  }
}

Socket::~Socket() {
  try {
    close_zmq_socket(socket_);
  } catch (exception& ex) {
    cerr << "Socket wasn't closed: " << ex.what() << endl;
  }
}

void Socket::send(const Message& message) {
  if (socket_type_ == SocketType::PUBLISHER) {
    send_zmq_msg(socket_, message);
  } else {
    throw logic_error("SUB socket can't send messages");
  }
}

Message Socket::receive() {
  if (socket_type_ == SocketType::SUBSCRIBER) {
    return get_zmq_msg(socket_);
  } else {
    throw logic_error("PUB socket can't receive messages");
  }
}

void Socket::subscribe(std::string endpoint) {
  if (socket_type_ == SocketType::SUBSCRIBER) {
    connect_zmq_socket(socket_, endpoint);
  } else {
    throw logic_error("Subscribe is only for SUB sockets");
  }
}

string Socket::endpoint() const {
  return endpoint_;
}