#include "client.h"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "m_zmq.h"
#include "md5sum.h"

using namespace std;

const int ERR_LOOP = 2;
const int MESSAGE_WAITING_TIME = 1;

const string FILES_FOLDER = "files/";

void* second_thread(void* cli_arg) {
  Client* client_ptr = (Client*)cli_arg;
  try {
    string endpoint = create_endpoint(EndpointType::SERVER_PUB_GENERAL);
    client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, move(endpoint));

    while (!client_ptr->terminated_) {
      shared_ptr<Message> msg_ptr = client_ptr->receive();
      if (msg_ptr->command == CommandType::ERROR) {
        if (client_ptr->terminated_) {
          return NULL;
        } else {
          cout << "Error" << endl;
          continue;
        }
      }
      if (msg_ptr->to_id != client_ptr->id()) {
        continue;
      }
      client_ptr->log("Message received by client: "s + msg_ptr->get_stats());
      switch (msg_ptr->command) {
        case CommandType::CONNECT:
          client_ptr->id_ = msg_ptr->value;
          client_ptr->server_is_avaible_ = true;

          endpoint = create_endpoint(EndpointType::CLIENT_PUB, client_ptr->id());
          client_ptr->publiser_ = nullptr;
          client_ptr->publiser_ = make_unique<Socket>(client_ptr->context_, SocketType::PUBLISHER, move(endpoint));

          endpoint = create_endpoint(EndpointType::SERVER_PUB, client_ptr->id());
          client_ptr->subscriber_ = nullptr;
          client_ptr->subscriber_ = make_unique<Socket>(client_ptr->context_, SocketType::SUBSCRIBER, move(endpoint));
          break;
        case CommandType::REGISTER:
        case CommandType::LOGIN:
          if (msg_ptr->value) {
            client_ptr->status = Client::Status::LOGGED;
          } else {
            client_ptr->status = Client::Status::LOG_ERROR;
          }
          break;
        case CommandType::CREATE_CHAT:
          if (msg_ptr->value) {
            cout << "You're in chat with: " << ((TextMessage*)msg_ptr.get())->text << endl;
            client_ptr->status = Client::Status::IN_CHAT;
          } else {
            cout << "Can't create chat" << endl;
          }
          break;
        case CommandType::LEFT_CHAT:
          cout << "Companion left the chat" << endl;
          cout << "Enter your companion username" << endl;
          client_ptr->leave_chat_actions();
          break;
        case CommandType::TEXT:
          cout << ">" << ((TextMessage*)msg_ptr.get())->text << endl;
          break;
        case CommandType::FILE_NAME: {
          if (!filesystem::exists(FILES_FOLDER)) {
            filesystem::create_directory(FILES_FOLDER);
          }
          string name = ((TextMessage*)msg_ptr.get())->text;
          cout << "Filename: " << name << endl;
          client_ptr->fout.open(FILES_FOLDER + name, ios::binary);
          if (!client_ptr->fout.is_open()) {
            throw runtime_error("Cannot open a file");
          }
          break;
        }
        case CommandType::FILE_PART: {
          FileMessage& file_msg = *(FileMessage*)msg_ptr.get();
          client_ptr->fout.write(reinterpret_cast<char*>(file_msg.buf), file_msg.size);
          if (file_msg.value == FileMessage::LAST_PART) {
            client_ptr->fout.close();
            cout << "File is received" << endl;
          }
          break;
        }
        default:
          throw logic_error("Undefined command type");
          break;
      }
    }

  } catch (exception& ex) {
    client_ptr->log("Client exctption: "s + ex.what() + "\nTerminated by exception on client receive loop"s);
    cout << "Terminated by error on loop" << endl;
    exit(ERR_LOOP);
  }
  return NULL;
}

Client::Client() {
  log("Starting client...");
  context_ = create_zmq_context();

  if (!filesystem::exists(ENDPOINT_FOLDER)) {
    filesystem::create_directory(ENDPOINT_FOLDER);
  }

  string endpoint = create_endpoint(EndpointType::SERVER_SUB_GENERAL);
  publiser_ = make_unique<Socket>(context_, SocketType::PUBLISHER, move(endpoint));

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
  disconnect_from_server();
  log("Destroying client...");
  terminated_ = true;

  try {
    publiser_ = nullptr;
    subscriber_ = nullptr;
    destroy_zmq_context(context_);
    pthread_join(second_thread_id_, NULL);
  } catch (exception& ex) {
    log("Client wasn't destroyed: "s + ex.what());
  }
}

int Client::id() const {
  return id_;
}

void Client::send(shared_ptr<Message> message) {
  publiser_->send(message);
  log("Message sended from client: "s + message->get_stats());
}

shared_ptr<Message> Client::receive() {
  return subscriber_->receive();
}

void Client::log(string message) {
  logger_.log(move(message));
}

void Client::connect_to_server() {
  while (!server_is_avaible_) {
    cout << "Trying to connect to the server..." << endl;
    send(Message::connect_message(id_));
    sleep(MESSAGE_WAITING_TIME);
  }
  cout << "Connected to server" << endl;
}

void Client::disconnect_from_server() {
  cout << "Disconnecting from the server..." << endl;
  send(Message::disconnect_message(id_));
}

const int LOGIN_CHECK_COUNT = 5;
void Client::login_form() {
  cout << "Enter login and password" << endl;
  string uname, log, pas;
  if (!(cin >> log >> pas)) {
    throw runtime_error("Incorrect input");
  }
  status = Client::Status::UNLOGGED;
  send(make_shared<TextMessage>(CommandType::LOGIN, id_, 0, md5sum(log) + " "s + md5sum(pas)));
  int cnt = LOGIN_CHECK_COUNT;
  while (cnt-- > 0 && status == Client::Status::UNLOGGED) {
    cout << "Checking..." << endl;
    sleep(1);
  }
  if (status == Client::Status::LOG_ERROR || status == Client::Status::UNLOGGED) {
    cout << "Please, try again" << endl;
  }
}

void Client::register_form() {
  cout << "Enter username, login and password" << endl;
  string uname, log, pas;
  if (!(cin >> uname >> log >> pas)) {
    throw runtime_error("Incorrect input");
  }
  status = Client::Status::UNLOGGED;
  send(make_shared<TextMessage>(CommandType::REGISTER, id_, 0, uname + " "s + md5sum(log) + " "s + md5sum(pas)));
  int cnt = LOGIN_CHECK_COUNT;
  while (cnt-- > 0 && status == Client::Status::UNLOGGED) {
    cout << "Checking..." << endl;
    sleep(1);
  }
  if (status == Client::Status::LOG_ERROR || status == Client::Status::UNLOGGED) {
    cout << "Please, try again" << endl;
  }
}

void Client::enter_in_system() {
  while (status != Client::Status::LOGGED) {
    cout << "Do you have an account? (y/n)" << endl;
    string str;
    if (!(cin >> str)) {
      throw runtime_error("Incorrect input");
    }
    if (str == "y") {
      login_form();
    } else if (str == "n") {
      register_form();
    } else {
      cout << "Please, answer 'y' or 'n'" << endl;
    }
  }
  cout << "Autorized" << endl;
}

void Client::send_text_msg(string message) {
  send(make_shared<TextMessage>(CommandType::TEXT, id_, 0, move(message)));
}

void Client::send_file_part_msg(const vector<uint8_t>& file_part, int value) {
  send(make_shared<FileMessage>(CommandType::FILE_PART, id_, 0, value, move(file_part)));
}

void Client::send_file_msg(filesystem::path path) {
  if (!filesystem::is_regular_file(path)) {
    cout << "No such file" << endl;
    return;
  }
  send(make_shared<TextMessage>(CommandType::FILE_NAME, id_, 0, path.filename()));
  fin.open(path, ios::binary);
  if (!fin.is_open()) {
    cout << "Cannot open a file" << endl;
    return;
  }
  vector<uint8_t> vec(FileMessage::BUF_SIZE);
  size_t file_size = filesystem::file_size(path);
  while (fin) {
    fin.read(reinterpret_cast<char*>(vec.data()), FileMessage::BUF_SIZE);
    vec.resize(fin.gcount());

    if (fin) {
      send_file_part_msg(vec, FileMessage::COMMON_PART);
    } else {
      send_file_part_msg(vec, FileMessage::LAST_PART);
    }
  }
  fin.close();
  cout << "File is sent" << endl;
}

void Client::enter_chat(string uname) {
  cout << "Trying to create chat whith " << uname << "..." << endl;
  send(make_shared<TextMessage>(CommandType::CREATE_CHAT, id_, 0, move(uname)));
}

void Client::leave_chat_actions() {
  status = Client::Status::LOGGED;
}

void Client::leave_chat() {
  cout << "Exiting from chat..." << endl;
  send(Message::left_chat_message(id_));
  leave_chat_actions();
  cout << "Enter your companion username" << endl;
}
