#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

enum class MessageType {
  BASIC,
  TEXT,
  FILE
};

enum class CommandType {
  ERROR,
  CONNECT,
  DISCONNECT,
  TEXT,
  REGISTER,
  LOGIN,
  CREATE_CHAT,
  LEFT_CHAT,
  FILE_NAME,
  FILE_PART
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
  static std::shared_ptr<Message> left_chat_message(int id);
};

class TextMessage : public Message {
 public:
  static const size_t MAX_MESSAGE_SIZE = 1024;

  char text[MAX_MESSAGE_SIZE + 1];

  TextMessage();
  TextMessage(CommandType command, int from_id, int to_id, const std::string& text, int value = 0);
};

class FileMessage : public Message {
 public:
  static const int COMMON_PART = 0;
  static const int LAST_PART = 1;

  static const size_t BUF_SIZE = 1000000;

  uint8_t buf[BUF_SIZE];
  size_t size;

  FileMessage(CommandType command, int from_id, int to_id, int value, const std::vector<uint8_t>& buf_vec);
};