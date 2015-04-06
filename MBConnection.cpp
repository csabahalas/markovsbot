#include <iostream>
#include <mysql.h>
#include "MBConnection.hpp"

MBConnection::MBConnection(const std::string& user, 
  const std::string& password, 
  const std::string& host) : _conn(0), _user(user), _password(password), _host(host)
{
  static int id = 0;
  this->_id = id++;
}

void MBConnection::open()
{
  if (this->_conn)
    return;
  
  this->_conn = mysql_init(nullptr);
  if (!mysql_real_connect(this->_conn, this->_host.c_str(), this->_user.c_str(), this->_password.c_str(), nullptr, 0, nullptr, 0))
  {
    std::cerr << "Connect failed on " << this->_user << "@" << this->_host << " using " << this->_password << std::endl;
    throw 0;
  }
}

void MBConnection::close()
{
  if (this->_conn)
    mysql_close(this->_conn);
}

////////////////////////