#include "client.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

#include "m_zmq.h"

using namespace std;

const int ERR_LOOP = 2;
const int MESSAGE_WAITING_TIME = 1;

void* second_thread(void* cli_arg) {
  Client* client_ptr = (Client*)cli_arg;
  try {
    string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
    client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, endpoint);

    while (true) {
      Message msg = client_ptr->receive();
      if (msg.command == CommandType::ERROR) {
        if (client_ptr->terminated_) {
          return NULL;
        } else {
          throw runtime_error("Can't receive message");
        }
      }
      if (msg.to_id != client_ptr->id()) {
        continue;
      }
      client_ptr->log("Message received by client: "s + msg.get_stats());
      switch (msg.command) {
        case CommandType::CONNECT:
          pthread_mutex_lock(&client_ptr->socket_change_mutex_);
          client_ptr->id_ = msg.value;
          client_ptr->server_is_avaible_ = true;

          endpoint = create_endpoint(EndpointType::CLIENT_PUB, client_ptr->id());
          client_ptr->publiser_ = nullptr;
          client_ptr->publiser_ = make_unique<Socket>(client_ptr->context_, SocketType::PUBLISHER, endpoint);

          endpoint = create_endpoint(EndpointType::SERVER_PUB, client_ptr->id());
          client_ptr->subscriber_ = nullptr;
          client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, endpoint);

          pthread_mutex_unlock(&client_ptr->socket_change_mutex_);
          break;

        default:
          throw logic_error("Undefined command type");
          break;
      }
    }

  } catch (exception& ex) {
    client_ptr->log("Client exctption: "s + ex.what() + "\nTerminated by exception on client receive loop"s);
    exit(ERR_LOOP);
  }
  return NULL;
}

void Client::refresh_publisher() {
  pthread_mutex_lock(&socket_change_mutex_);
  static string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = nullptr;
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);
  pthread_mutex_unlock(&socket_change_mutex_);
}

void Client::connect_to_server() {
  while (!server_is_avaible_) {
    //refresh_publisher();
    cout << "Trying to connect to the server..." << endl;
    send(Message::connect_message(id_));
    sleep(MESSAGE_WAITING_TIME);
  }
  cout << "Connected to server" << endl;
}

Client::Client() {
  log("Starting client...");
  context_ = create_zmq_context();

  string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, endpoint);

  if (pthread_mutex_init(&socket_change_mutex_, NULL) != 0) {
    throw runtime_error("Can't init mutex");
  }
  if (pthread_create(&second_thread_id_, 0, second_thread, this) != 0) {
    cout << "Can't run second thread" << endl;
    exit(ERR_LOOP);
  }

  srand(time(NULL) + clock());
  id_ = rand();
}

Client::~Client() {
  if (terminated_) {
    log("Client double termination");
    return;
  }

  log("Destroying client...");
  terminated_ = true;

  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);

    if (pthread_mutex_destroy(&socket_change_mutex_) != 0) {
      throw runtime_error("Can't destroy mutex");
    }
  } catch (exception& ex) {
    log("Client wasn't destroyed: "s + ex.what());
  }
}

int Client::id() const {
  return id_;
}

void Client::send(const Message& message) {
  publiser_->send(message);
  log("Message sended from client: "s + message.get_stats());
}

Message Client::receive() {
  return subscriber_->receive();
}

void Client::log(string message) {
  logger_.log(move(message));
}