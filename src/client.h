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

  void log(std::string message);
  void connect_to_server();
  void disconnect_from_server();

  void enter();
  void register_form();
  void login_form();

  void send_text_msg(std::string message);

  int id() const;

  friend void* second_thread(void* cli_arg);

 private:
  int id_ = -1;
  void* context_ = nullptr;
  std::unique_ptr<Socket> publiser_;
  std::unique_ptr<Socket> subscriber_;

  pthread_t second_thread_id_;
  bool server_is_avaible_ = false;

  bool terminated_ = false;
  Logger logger_ = Logger(/*"log.txt"*/);

  enum class Status {
    UNLOGGED,
    LOGGED,
    LOG_ERROR
  };
  Status status_ = Status::UNLOGGED;

  void send(std::shared_ptr<Message> message);
  std::shared_ptr<Message> receive();
};