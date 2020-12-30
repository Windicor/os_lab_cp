#pragma once

#include <fstream>
#include <optional>
#include <string>

#include "m_zmq.h"

class Logger {
 public:
  Logger();
  explicit Logger(std::string filename);
  void log(std::string message);

 private:
  std::optional<std::ofstream> fout_opt;
};
