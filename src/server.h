#pragma once

#include <unistd.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "logger.h"
#include "security.h"
#include "socket.h"

class Online {
 public:
  void add_user(std::string username, int id);
  void remove_user(int id);
  std::optional<int> get_id(std::string username) const;
  std::optional<std::string> get_username(int id) const;
  bool check_username(std::string username) const;

 private:
  std::unordered_map<int, std::string> id_to_username_;
  std::unordered_map<std::string, int> username_to_id_;
};

class Server {
 public:
  Server();
  ~Server();

  std::shared_ptr<Message> receive();

  void log(std::string message);
  void add_connection(int id);
  void remove_connection(int id);
  void register_form(std::shared_ptr<Message> msg_ptr);
  void login_form(std::shared_ptr<Message> msg_ptr);

 private:
  void* context_ = nullptr;
  std::unique_ptr<Socket> subscriber_;
  std::unique_ptr<Socket> general_publiser_;
  std::unordered_map<int, std::unique_ptr<Socket>> id_to_publisher_;
  Online online_;

  int id_cntr = 0;

  Logger logger_;
  Security security;

  void send_to_general(std::shared_ptr<Message> message);
  void send_to_user(int id, std::shared_ptr<Message> message);
  // void send_from_user_to_user(int from_id, int to_id, std::shared_ptr<Message> message);
};
