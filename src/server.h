#pragma once

#include <unistd.h>

#include <memory>

#include "logger.h"
#include "socket.h"

class Server {
 public:
  Server();
  ~Server();

  void send(const Message& message);
  Message receive();

  void log(std::string message);

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;

  Logger logger_;
};
