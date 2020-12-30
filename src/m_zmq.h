#pragma once

#include <memory>
#include <string>

#include "message.h"

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

void send_zmq_msg(void* socket, std::shared_ptr<Message> msg);
std::shared_ptr<Message> get_zmq_msg(void* socket);