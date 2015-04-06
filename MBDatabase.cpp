#include <algorithm>
#include "MBConnection.hpp"
#include "MBDatabase.hpp"

MBConnection MBDatabase::newConnection()
{
  MBConnection conn(this->_user, this->_password, this->_host);  
  conn.open();

  return conn;
}