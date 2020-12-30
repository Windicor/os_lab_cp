#include "m_zmq.h"

#include <unistd.h>
#include <zmq.h>

#include <algorithm>
#include <cstring>
#include <iostream>

using namespace std;

void* create_zmq_context() {
  void* context = zmq_ctx_new();
  if (context == NULL) {
    throw runtime_error("Can't create new context. pid:" + to_string(getpid()));
  }
  return context;
}

void destroy_zmq_context(void* context) {
  if (zmq_ctx_destroy(context) != 0) {
    throw runtime_error("Can't destroy context. pid:" + to_string(getpid()));
  }
}

int get_zmq_socket_type(SocketType type) {
  switch (type) {
    case SocketType::PUBLISHER:
      return ZMQ_PUB;
    case SocketType::SUBSCRIBER:
      return ZMQ_SUB;
    default:
      throw logic_error("Undefined socket type");
  }
}

void* create_zmq_socket(void* context, SocketType type) {
  int zmq_type = get_zmq_socket_type(type);
  void* socket = zmq_socket(context, zmq_type);
  if (socket == NULL) {
    throw runtime_error("Can't create socket");
  }
  if (zmq_type == ZMQ_SUB) {
    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, 0, 0) == -1) {
      throw runtime_error("Can't set ZMQ_SUBSCRIBE option");
    }
  }
  int linger_period = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_period, sizeof(int)) == -1) {
    throw runtime_error("Can't set ZMQ_LINGER option");
  }
  return socket;
}

void close_zmq_socket(void* socket) {
  sleep(1);  // Don't comment it, because sometimes zmq_close blocks
  if (zmq_close(socket) != 0) {
    throw runtime_error("Can't close socket");
  }
}

string create_endpoint(EndpointType type, int id) {
  switch (type) {
    case EndpointType::SERVER_PUB_GENERAL:
      return "ipc://tmp/server_pub_general"s;
    case EndpointType::SERVER_SUB_GENERAL:
      return "ipc://tmp/server_sub_general"s;
    case EndpointType::SERVER_PUB:
      return "ipc://tmp/server_pub_"s + to_string(id);
    case EndpointType::CLIENT_PUB:
      return "ipc://tmp/client_pub_"s + to_string(id);
    default:
      throw logic_error("Undefined endpoint type");
  }
}

void bind_zmq_socket(void* socket, string endpoint) {
  if (zmq_bind(socket, endpoint.data()) != 0) {
    throw runtime_error("Can't bind socket (create 'tmp' folder)");
  }
}

void unbind_zmq_socket(void* socket, string endpoint) {
  if (zmq_unbind(socket, endpoint.data()) != 0) {
    throw runtime_error("Can't unbind socket");
  }
}

void connect_zmq_socket(void* socket, string endpoint) {
  if (zmq_connect(socket, endpoint.data()) != 0) {
    throw runtime_error("Can't connect socket (create 'tmp' folder)");
  }
}

void disconnect_zmq_socket(void* socket, string endpoint) {
  if (zmq_disconnect(socket, endpoint.data()) != 0) {
    throw runtime_error("Can't disconnect socket");
  }
}

void create_zmq_msg(zmq_msg_t* zmq_msg, shared_ptr<Message> msg_ptr) {
  switch (msg_ptr->type()) {
    case MessageType::BASIC:
      zmq_msg_init_size(zmq_msg, sizeof(Message));
      *(Message*)zmq_msg_data(zmq_msg) = *(Message*)msg_ptr.get();
      break;
    case MessageType::TEXT:
      zmq_msg_init_size(zmq_msg, sizeof(TextMessage));
      *(TextMessage*)zmq_msg_data(zmq_msg) = *(TextMessage*)msg_ptr.get();
      break;
    default:
      throw logic_error("Unemplemented message type");
  }
}

void send_zmq_msg(void* socket, shared_ptr<Message> msg_ptr) {
  zmq_msg_t zmq_msg;
  create_zmq_msg(&zmq_msg, msg_ptr);
  if (!zmq_msg_send(&zmq_msg, socket, 0)) {
    throw runtime_error("Can't send message");
  }
  zmq_msg_close(&zmq_msg);
}

shared_ptr<Message> get_zmq_msg(void* socket) {
  zmq_msg_t zmq_msg;
  zmq_msg_init(&zmq_msg);
  if (zmq_msg_recv(&zmq_msg, socket, 0) == -1) {
    return Message::error_message();
  }
  shared_ptr<Message> msg_ptr = make_shared<Message>(*(Message*)zmq_msg_data(&zmq_msg));

  switch (msg_ptr->type()) {
    case MessageType::BASIC:
      break;
    case MessageType::TEXT: {
      msg_ptr = make_shared<TextMessage>(*(TextMessage*)zmq_msg_data(&zmq_msg));
      break;
    }
    default:
      throw logic_error("Unemplemented message type");
  }

  zmq_msg_close(&zmq_msg);
  return msg_ptr;
}