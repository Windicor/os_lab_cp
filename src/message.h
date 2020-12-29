#pragma once

#include <string>

enum class CommandType {
  ERROR,
  PING,
};

struct Message {
  CommandType command = CommandType::ERROR;
  int from_id;
  int to_id;

  Message() = default;
  Message(CommandType command, int from_id, int to_id);

  std::string get_stats() const;

  static Message error_message();
  static Message ping_message();
};

struct TextMessage : Message {
  static const size_t MAX_MESSAGE_SIZE = 1024;
  char text[MAX_MESSAGE_SIZE + 1];

  TextMessage();
  TextMessage(CommandType command, int from_id, int to_id, std::string text);
};