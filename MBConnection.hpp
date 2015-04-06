#ifndef _MB_CONNECTION_HPP_
#define _MB_CONNECTION_HPP_

#include <mysql.h>
#include <string>

class MBConnection
{
public:
  MBConnection(const std::string& user, 
    const std::string& password, 
    const std::string& host);

  // boost python doesn't support move semantics
  void open();
  void close();

  void 

  int getId() const { return this->_id; }  
private:
  MYSQL* _conn;
  int _id;
  std::string _user;
  std::string _password;
  std::string _host;
};

#endif