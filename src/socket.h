#pragma once

#include <string>

#include "m_zmq.h"

enum class ConnectionType {
  BIND,
  CONNECT
};

class Socket {
 public:
  Socket(void* context, SocketType socket_type, std::string endpoint);
  ~Socket();

  void send(const Message& message);
  Message receive();

  void subscribe(std::string endpoint);
  void unsubscribe(std::string endpoint);
  std::string endpoint() const;

 private:
  void* socket_;
  SocketType socket_type_;
  std::string endpoint_;
};