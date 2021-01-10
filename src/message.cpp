#include "message.h"

#include <unistd.h>

#include <cstring>
#include <stdexcept>

using namespace std;

Message::Message(CommandType command, int from_id, int to_id, int value)
    : command(command),
      from_id(from_id),
      to_id(to_id),
      value(value) {
}

std::string Message::get_stats() const {
  return to_string(static_cast<int>(command)) + " " + to_string(from_id) + " " + to_string(to_id) + " " + to_string(value);
}
shared_ptr<Message> Message::error_message() {
  return make_shared<Message>(CommandType::ERROR, 0, 0, 0);
}
shared_ptr<Message> Message::connect_message(int id) {
  return make_shared<Message>(CommandType::CONNECT, id, 0, 0);
}
shared_ptr<Message> Message::disconnect_message(int id) {
  return make_shared<Message>(CommandType::DISCONNECT, id, 0, 0);
}
shared_ptr<Message> Message::left_chat_message(int id) {
  return make_shared<Message>(CommandType::LEFT_CHAT, id, 0, 0);
}

TextMessage::TextMessage() {
  type_ = MessageType::TEXT;
  text[0] = '\0';
}
TextMessage::TextMessage(CommandType command, int from_id, int to_id, string text_str, int value)
    : Message(command, from_id, to_id, value) {
  type_ = MessageType::TEXT;
  if (text_str.size() > MAX_MESSAGE_SIZE) {
    throw logic_error("Message text can't be longer, than MAX_MESSAGE_SIZE");
  }
  memcpy(text, text_str.data(), text_str.size() + 1);
}
