#ifndef _MB_DATABASE_HPP_
#define _MB_DATABASE_HPP_

#include <string>
#include <utility>
#include <vector>
#include "MBConnection.hpp"

class MBDatabase
{
public:
  MBDatabase(const std::string& user, const std::string& password, const std::string& host);  
  MBConnection newConnection();     

private:
  std::string _user;
  std::string _password;
  std::string _host;  
};

#endif