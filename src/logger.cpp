#include "logger.h"

#include <unistd.h>

#include <iostream>

using namespace std;

Logger::Logger() {}

Logger::Logger(std::string filename) {
  fout.open(filename);
  if (!fout) {
    throw runtime_error("Can't create log file");
  }
}

void Logger::log(std::string message) {
  message = to_string(getpid()) + ": " + message + "\n";
  if (fout.is_open()) {
    fout << message;
    fout.flush();
  } else {
    cerr << message;
    cerr.flush();
  }
}