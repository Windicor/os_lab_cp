#include "security.h"

#include <fstream>

using namespace std;

const string SECURITY_FILENAME = "log_n_pass_database.txt";

bool operator==(const LogAndPas& lhs, const LogAndPas& rhs) {
  return lhs.login == rhs.login && lhs.password == rhs.password;
}

size_t LogAndPasHasher::operator()(const LogAndPas& key) const {
  return std::hash<std::string>()(key.login) + std::hash<std::string>()(key.password);
}

istream& operator>>(istream& is, LogAndPas& lap) {
  is >> lap.login >> lap.password;
  return is;
}

ostream& operator<<(ostream& os, const LogAndPas& lap) {
  os << lap.login << "\n"
     << lap.password;
  return os;
}

Security::Security() {
  ifstream is(SECURITY_FILENAME);
  if (!is) {
    return;
  }
  string uname;
  LogAndPas lap;
  while (is >> uname >> lap) {
    uname_to_lap_[uname] = lap;
    lap_to_uname_[move(lap)] = move(uname);
  }
}

Security::~Security() {
  ofstream os(SECURITY_FILENAME);
  if (!os) {
    cerr << "Can't open log_n_pass file" << endl;
  }
  for (const auto& [uname, lap] : uname_to_lap_) {
    os << uname << "\n"
       << lap << "\n";
  }
}

bool Security::Register(string username, LogAndPas log_n_pass) {
  auto it = uname_to_lap_.find(username);
  if (it != uname_to_lap_.end()) {
    return false;
  }
  auto it2 = lap_to_uname_.find(log_n_pass);
  if (it2 != lap_to_uname_.end()) {
    return false;
  }
  uname_to_lap_[username] = log_n_pass;
  lap_to_uname_[move(log_n_pass)] = move(username);
  return true;
}

optional<string> Security::get_username(LogAndPas log_n_pass) const {
  auto it = lap_to_uname_.find(move(log_n_pass));
  if (it != lap_to_uname_.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}
