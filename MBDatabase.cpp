#include <algorithm>
#include <mysql.h>
#include "MBConnection.hpp"
#include "MBDatabase.hpp"

MBDatabase::MBDatabase(const std::string& user, const std::string& password, const std::string& host)
: _user(user), _password(password), _host(host)
{
  MBConnection conn(this->_user, this->_password, this->_host);  
  conn.open(); 

  conn.createDatabase();
  conn.createTables();

  conn.close();
}

MBConnection MBDatabase::newConnection()
{
  MBConnection conn(this->_user, this->_password, this->_host);  
  conn.open(); 
  
  conn.createPreparedStatements();

  return conn;
}