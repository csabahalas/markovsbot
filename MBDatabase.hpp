#ifndef _MB_DATABASE_HPP_
#define _MB_DATABASE_HPP_

#include <string>
#include <utility>
#include <vector>
#include "MBConnection.hpp"

class MBDatabase
{
public:
  MBDatabase(std::string user, std::string password, std::string host)
   : _user(std::move(user)), _password(std::move(password)), _host(std::move(host)) {}
  
  MBConnection newConnection();   

private:
  std::string _user;
  std::string _password;
  std::string _host;  
};

#endif