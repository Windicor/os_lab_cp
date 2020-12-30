#pragma once

#include <memory>
#include <string>

enum class MessageType {
  BASIC,
  TEXT
};

enum class CommandType {
  ERROR,
  CONNECT,
  DISCONNECT,
  TEXT
};

class Message {
 protected:
  MessageType type_ = MessageType::BASIC;

 public:
  CommandType command = CommandType::ERROR;
  int from_id = 0;
  int to_id = 0;
  int value = 0;

  Message() = default;
  Message(CommandType command, int from_id, int to_id, int value);
  virtual ~Message() = default;

  std::string get_stats() const;
  MessageType type() const {
    return type_;
  }

  static std::shared_ptr<Message> error_message();
  static std::shared_ptr<Message> connect_message(int id);
  static std::shared_ptr<Message> disconnect_message(int id);
};

class TextMessage : public Message {
 protected:
  static const size_t MAX_MESSAGE_SIZE = 1024;

 public:
  char text[MAX_MESSAGE_SIZE + 1];

  TextMessage();
  TextMessage(CommandType command, int from_id, int to_id, std::string text);
};