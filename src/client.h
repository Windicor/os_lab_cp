#pragma once

#include <unistd.h>

#include <memory>
#include <string>

#include "socket.h"

class Client {
 public:
  Client();
  ~Client();

  void send(const Message& message);
  Message receive();

  friend void* second_thread(void* cli_arg);

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;

  pthread_t second_thread_id;
  bool terminated_ = false;
};