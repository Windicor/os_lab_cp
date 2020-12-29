#pragma once

#include <unistd.h>

#include <memory>

#include "socket.h"

class Server {
 public:
  Server();
  ~Server();

  void send(Message message);
  Message receive();

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;
};
