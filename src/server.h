#pragma once

#include <unistd.h>

#include <memory>
#include <unordered_map>

#include "logger.h"
#include "socket.h"

class Server {
 public:
  Server();
  ~Server();

  void send(std::shared_ptr<Message> message);
  std::shared_ptr<Message> receive();

  void log(std::string message);
  void add_connection(int id);
  void remove_connection(int id);

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> subscriber_;
  std::unique_ptr<Socket> general_publiser_;
  std::unordered_map<int, std::unique_ptr<Socket>> id_to_publisher_;
  int id_cntr = 0;

  Logger logger_;
};
