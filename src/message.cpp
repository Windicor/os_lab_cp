#include "message.h"

#include <unistd.h>

#include <cstring>
#include <stdexcept>

using namespace std;

Message::Message(CommandType command, int from_id, int to_id)
    : command(command),
      from_id(from_id),
      to_id(to_id) {
}
std::string Message::get_stats() const {
  return to_string(static_cast<int>(command)) + " " + to_string(from_id) + " " + to_string(to_id);
}
Message Message::error_message() {
  return Message(CommandType::ERROR, 0, 0);
}
Message Message::ping_message() {
  return Message(CommandType::PING, 0, 0);
}

TextMessage::TextMessage() {
  text[0] = '\0';
}
TextMessage::TextMessage(CommandType command, int from_id, int to_id, string text_str)
    : Message(command, from_id, to_id) {
  if (text_str.size() > MAX_MESSAGE_SIZE) {
    throw logic_error("Message text can't be longer, than MAX_MESSAGE_SIZE");
  }
  memcpy(text, text_str.data(), text_str.size() + 1);
}
