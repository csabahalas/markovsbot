#include <iostream>
#include <mysql.h>
#include <cstring>
#include "MBConnection.hpp"
#include "MySqlHelper.hpp"

MBConnection::MBConnection(const std::string& user, 
  const std::string& password, 
  const std::string& host) : _conn(0), _user(user), _password(password), _host(host), 
  _insPlayerRowStatement(0),
  _insWordRowStatement(0),
  _insConvRowStatement(0)
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
    std::cout << "Connect failed on " << this->_user << "@" << this->_host << " using " << this->_password << std::endl;
    throw 0;
  }  
}

void MBConnection::createPreparedStatements()
{
  this->_insPlayerRowStatement = mysql_stmt_init(this->_conn);
  this->_insWordRowStatement = mysql_stmt_init(this->_conn);
  this->_insConvRowStatement = mysql_stmt_init(this->_conn);

  if (!this->_insPlayerRowStatement || !this->_insWordRowStatement || !this->_insConvRowStatement)
  {
    std::cout << "Initializating prepared statements failed" << std::endl;
    throw 0;
  }

  static const std::string INSERT_PLAYER_ROW_SQL("INSERT INTO markovsbot.player_names (id,name) VALUES (?,?)");
  static const std::string INSERT_WORD_ROW_SQL("INSERT INTO markovsbot.words (id,word) VALUES (?,?)");
  static const std::string INSERT_CONV_ROW_SQL("INSERT INTO markovsbot.conversations (name_id, word_a, word_b, is_starting) VALUES (?,?,?,?)");

  int r1 = mysql_stmt_prepare(this->_insPlayerRowStatement, INSERT_PLAYER_ROW_SQL.c_str(), INSERT_PLAYER_ROW_SQL.length());
  int r2 = mysql_stmt_prepare(this->_insWordRowStatement, INSERT_WORD_ROW_SQL.c_str(), INSERT_WORD_ROW_SQL.length());
  int r3 = mysql_stmt_prepare(this->_insConvRowStatement, INSERT_CONV_ROW_SQL.c_str(), INSERT_CONV_ROW_SQL.length());

  if (r1 || r2 || r3)
  {
    std::cout << "Creating prepared statements failed" << std::endl;
    throw 0;
  }
}

void MBConnection::close()
{
  if (this->_conn)
  {
    if (this->_insPlayerRowStatement)
      mysql_stmt_close(this->_insPlayerRowStatement);

    if (this->_insWordRowStatement)
      mysql_stmt_close(this->_insWordRowStatement);

    if (this->_insConvRowStatement)
      mysql_stmt_close(this->_insConvRowStatement);
    mysql_close(this->_conn);
  }
}

void MBConnection::createDatabase()
{
  if (mysql_query(this->_conn, "CREATE DATABASE IF NOT EXISTS markovsbot"))
  {
    std::cout << "Couldn't create database" << std::endl;
    throw 0;
  }  
}

void MBConnection::createTables()
{
  int r1 = mysql_query(this->_conn, "CREATE TABLE IF NOT EXISTS markovsbot.player_names ("
    "id INT PRIMARY KEY AUTO_INCREMENT NOT NULL,"
    "name CHAR(24) NOT NULL,"
    "INDEX name_i (name)) ENGINE=INNODB");

  int r2 = mysql_query(this->_conn, "CREATE TABLE IF NOT EXISTS markovsbot.words (id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, "
    "word CHAR(32) NOT NULL, INDEX word_i (word)) ENGINE=INNODB");

  int r3 = mysql_query(this->_conn, "CREATE TABLE IF NOT EXISTS markovsbot.conversations "
    "(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, name_id INT NOT NULL, word_a INT NOT NULL, word_b INT NOT NULL, is_starting TINYINT NOT NULL,"
      "FOREIGN KEY word_a_fk (word_a) REFERENCES words (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
      "FOREIGN KEY word_b_fk (word_b) REFERENCES words (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
      "FOREIGN KEY name_fk (name_id) REFERENCES player_names (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
      "INDEX name_i (name_id), INDEX wa_i (word_a)) ENGINE=INNODB");

  if (r1 || r2 || r3)
  {
    std::cout << "Error creating tables" << std::endl;
    throw 0;
  }
}

void MBConnection::insertPlayerRow(long id, const std::string& name)
{
  MYSQL_BIND param[2];
  memset(param, 0, 2 * sizeof(MYSQL_BIND));

  setBindValue<0>(param, id);
  setBindValue<1>(param, name);
  
  if (mysql_stmt_bind_param(this->_insPlayerRowStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_insPlayerRowStatement))
  {
    std::cout << "Executing insert player (id, name) failed" << std::endl;
    throw 0;
  }  
}

void MBConnection::insertWordRow(long id, const std::string& word)
{
  MYSQL_BIND param[2];
  memset(param, 0, 2 * sizeof(MYSQL_BIND));

  setBindValue<0>(param, id);
  setBindValue<1>(param, word);

  if (mysql_stmt_bind_param(this->_insWordRowStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_insWordRowStatement))
  {
    std::cout << "Executing insert words (id, word) failed" << std::endl;
    throw 0;
  }
}

void MBConnection::insertConvRow(long playerId, long wordId1, long wordId2, int isStarting)
{
  MYSQL_BIND param[4];
  memset(param, 0, 4 * sizeof(MYSQL_BIND));
  char iss = static_cast<char>(isStarting);

  setBindValue<0>(param, playerId);
  setBindValue<1>(param, wordId1);
  setBindValue<2>(param, wordId2);
  setBindValue<3>(param, iss);  

  if (mysql_stmt_bind_param(this->_insConvRowStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_insConvRowStatement))
  {
    std::cout << "Executing insert conv (player, word1, word2, isStarting) failed" << std::endl;
    throw 0;
  }
}

////////////////////////
