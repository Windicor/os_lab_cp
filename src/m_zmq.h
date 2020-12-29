#pragma once

#include <string>

void* create_zmq_context();
void destroy_zmq_context(void* context);

enum class SocketType {
  PUBLISHER,
  SUBSCRIBER
};
void* create_zmq_socket(void* context, SocketType type);
void close_zmq_socket(void* socket);

enum class EndpointType {
  SERVER_PUB_GENERAL,
  SERVER_SUB_GENERAL,
  SERVER_PUB,
  CLIENT_PUB
};
std::string create_endpoint(EndpointType type, int id = 0);

void bind_zmq_socket(void* socket, std::string endpoint);
void unbind_zmq_socket(void* socket, std::string endpoint);
void connect_zmq_socket(void* socket, std::string endpoint);
void disconnect_zmq_socket(void* socket, std::string endpoint);

enum class CommandType {
  ERROR,
  RETURN,
};

struct Message {
  static const size_t MAX_MESSAGE_SIZE = 1024;

  CommandType command = CommandType::ERROR;
  int from_id;
  int to_id;
  char text[MAX_MESSAGE_SIZE + 1];

  Message();
  Message(CommandType command, int from_id, int to_id, std::string text);
};

void send_zmq_msg(void* socket, const Message& msg);
Message get_zmq_msg(void* socket);