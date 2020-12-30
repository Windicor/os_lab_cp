#include "logger.h"

#include <unistd.h>

#include <iostream>

using namespace std;

Logger::Logger() {}

Logger::Logger(std::string filename) {
  fout_opt = ofstream(filename);
  if (!*fout_opt) {
    throw runtime_error("Can't create log file");
  }
}

void Logger::log(std::string message) {
  message = to_string(getpid()) + ": " + message + "\n";
  if (fout_opt) {
    operator<<(*fout_opt, message);
    fout_opt->flush();
  } else {
    cerr << message;
    cerr.flush();
  }
}