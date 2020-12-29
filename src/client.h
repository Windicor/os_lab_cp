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
  void connect_to_server();

  int id() const;

  friend void* second_thread(void* cli_arg);

 private:
  int id_ = -1;
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;

  pthread_t second_thread_id_;
  bool server_is_avaible_ = false;
  pthread_mutex_t socket_change_mutex_;

  bool terminated_ = false;
  Logger logger_ = Logger(/*"log.txt"*/);

  void refresh_publisher();
};