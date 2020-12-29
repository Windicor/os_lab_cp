#include "logger.h"

#include <unistd.h>

#include <iostream>

using namespace std;

Logger::Logger() {}

Logger::Logger(std::string filename) {
  fout = ofstream(filename);
}

void Logger::log(std::string message) {
  message = to_string(getpid()) + ": " + message + "\n";
  if (fout) {
    operator<<(*fout, message);
    fout->flush();
  } else {
    cerr << message;
    cerr.flush();
  }
}