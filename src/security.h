#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

struct LogAndPas {
  std::string login;
  std::string password;
};

bool operator==(const LogAndPas& lhs, const LogAndPas& rhs);
std::istream& operator>>(std::istream& is, LogAndPas& lap);
std::ostream& operator<<(std::ostream& os, const LogAndPas& lap);

struct LogAndPasHasher {
  size_t operator()(const LogAndPas& key) const;
};

class Security {
 public:
  Security();
  ~Security();

  bool Register(std::string username, LogAndPas log_n_pass);
  std::optional<std::string> get_username(LogAndPas log_n_pass) const;

 private:
  std::unordered_map<LogAndPas, std::string, LogAndPasHasher> lap_to_uname_;
  std::unordered_map<std::string, LogAndPas> uname_to_lap_;
};