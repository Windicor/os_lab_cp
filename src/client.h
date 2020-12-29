#pragma once

#include <unistd.h>

#include <memory>
#include <string>

#include "logger.h"
#include "socket.h"

class Client {
 public:
  Client();
  ~Client();

  void send(const Message& message);
  Message receive();

  void log(std::string message);
  void check_server_availability();

  friend void* second_thread(void* cli_arg);

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;

  pthread_t second_thread_id;
  bool terminated_ = false;

  Logger logger_ = Logger(/*"log.txt"*/);

  bool server_is_avaible_ = false;
};