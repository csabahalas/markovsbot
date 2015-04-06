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

  void createDatabase();
  void createTables();
  void createPreparedStatements();

  // TODO type correctness
  void insertPlayerRow(long id, const std::string& name);  
  void insertWordRow(long id, const std::string& word);
  void insertConvRow(long playerId, long wordId1, long wordId2, int isStarting);

  int getId() const { return this->_id; }  
private:
  MYSQL* _conn;
  int _id;
  std::string _user;
  std::string _password;
  std::string _host;

  MYSQL_STMT* _insPlayerRowStatement;
  MYSQL_STMT* _insWordRowStatement;
  MYSQL_STMT* _insConvRowStatement;
};

#endif