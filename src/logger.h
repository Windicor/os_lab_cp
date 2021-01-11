#pragma once

#include <fstream>
#include <string>

#include "m_zmq.h"

class Logger {
 public:
  Logger();
  explicit Logger(std::string filename);
  void log(std::string message);

 private:
  std::ofstream fout;
};
