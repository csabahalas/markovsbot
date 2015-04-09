#include <boost/python/tuple.hpp>
#include <cstring>
#include <iostream>
#include <mysql.h>
#include "MBConnection.hpp"
#include "MySqlHelper.hpp"

using namespace boost::python;

MBConnection::MBConnection(const std::string& user, 
  const std::string& password, 
  const std::string& host) : _conn(0), _user(user), _password(password), _host(host), 
  _insPlayerRowStatement(0),
  _insWordRowStatement(0),
  _insConvRowStatement(0),
  _getRandomStartingWordStatement(0),
  _getRandomNextWordStatement(0),
  _getProperCaseStatement(0),
  _getBigramCountStatement(0),
  _getPlayerLineCountStatement(0),
  _getPlayerNameStatement(0),
  _insTextRowStatement(0),
  _getRandomChatMessageStatement(0)
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
  this->_getRandomStartingWordStatement = mysql_stmt_init(this->_conn);
  this->_getRandomNextWordStatement = mysql_stmt_init(this->_conn);
  this->_getProperCaseStatement = mysql_stmt_init(this->_conn);
  this->_getBigramCountStatement = mysql_stmt_init(this->_conn);  
  this->_getPlayerLineCountStatement = mysql_stmt_init(this->_conn);
  this->_getPlayerNameStatement = mysql_stmt_init(this->_conn);
  this->_insTextRowStatement = mysql_stmt_init(this->_conn);
  this->_getRandomChatMessageStatement = mysql_stmt_init(this->_conn);

  if (!this->_insPlayerRowStatement || 
    !this->_insWordRowStatement || 
    !this->_insConvRowStatement ||
    !this->_getRandomStartingWordStatement ||
    !this->_getRandomNextWordStatement ||
    !this->_getProperCaseStatement ||
    !this->_getBigramCountStatement ||
    !this->_getPlayerLineCountStatement ||
    !this->_getPlayerNameStatement ||
    !this->_insTextRowStatement ||
    !this->_getRandomChatMessageStatement)
  {
    std::cout << "Initializating prepared statements failed" << std::endl;
    throw 0;
  }

  static const std::string INSERT_PLAYER_ROW_SQL("INSERT INTO markovsbot.player_names (id,name) VALUES (?,?)");
  static const std::string INSERT_WORD_ROW_SQL("INSERT INTO markovsbot.words (id,word) VALUES (?,?)");
  static const std::string INSERT_CONV_ROW_SQL("INSERT INTO markovsbot.conversations (name_id, word_a, word_b, is_starting) VALUES (?,?,?,?)");

  static const std::string GET_RANDOM_STARTING_WORD_SQL("SELECT markovsbot.words.id, markovsbot.words.word FROM ("
    "(SELECT markovsbot.conversations.word_a FROM markovsbot.player_names INNER JOIN markovsbot.conversations "
      "ON markovsbot.player_names.id=markovsbot.conversations.name_id "
      "WHERE markovsbot.player_names.name=? AND markovsbot.conversations.is_starting=1) "
    "AS t1) "
    "INNER JOIN markovsbot.words ON markovsbot.words.id=t1.word_a ORDER BY RAND() LIMIT 1");

  static const std::string GET_RANDOM_NEXT_WORD_SQL("SELECT markovsbot.words.id, markovsbot.words.word FROM ("
    "(SELECT markovsbot.conversations.word_b FROM markovsbot.player_names INNER JOIN markovsbot.conversations "
      "ON markovsbot.player_names.id=markovsbot.conversations.name_id "
      "WHERE markovsbot.player_names.name=? AND markovsbot.conversations.word_a=?) "
    "AS t1) "
    "INNER JOIN markovsbot.words ON markovsbot.words.id=t1.word_b ORDER BY RAND() LIMIT 1");

  static const std::string GET_PROPER_CASE_SQL("SELECT markovsbot.player_names.name FROM markovsbot.player_names WHERE name=? LIMIT 1");
  static const std::string GET_BIGRAM_COUNT_SQL("SELECT name_id, COUNT(*) AS c FROM ("
      "SELECT * FROM markovsbot.words WHERE word=?"
    ") AS t1, ("
      "SELECT * FROM markovsbot.words WHERE word=?"
    ") AS t2 INNER JOIN markovsbot.conversations WHERE markovsbot.conversations.word_a=t1.id AND "
    "markovsbot.conversations.word_b=t2.id GROUP BY markovsbot.conversations.name_id ORDER BY c DESC LIMIT 20");
  static const std::string GET_PLAYER_LINE_COUNT_SQL("SELECT COUNT(*) FROM markovsbot.conversations WHERE name_id=? GROUP BY name_id");
  static const std::string GET_PLAYER_NAME_SQL("SELECT name FROM markovsbot.player_names WHERE id=?");
  static const std::string INSERT_TEXT_SQL("INSERT INTO markovsbot.fulltext_log (name, `text`) VALUES (?, ?)");
  static const std::string GET_RANDOM_CHAT_MESSAGE_SQL("SELECT `name`,`text` FROM markovsbot.fulltext_log WHERE MATCH(`text`) AGAINST (CONCAT('\"', ?, '\"') IN BOOLEAN MODE) ORDER BY RAND() DESC LIMIT 1");

  int r1 = mysql_stmt_prepare(this->_insPlayerRowStatement, INSERT_PLAYER_ROW_SQL.c_str(), INSERT_PLAYER_ROW_SQL.length());
  int r2 = mysql_stmt_prepare(this->_insWordRowStatement, INSERT_WORD_ROW_SQL.c_str(), INSERT_WORD_ROW_SQL.length());
  int r3 = mysql_stmt_prepare(this->_insConvRowStatement, INSERT_CONV_ROW_SQL.c_str(), INSERT_CONV_ROW_SQL.length());

  int r4 = mysql_stmt_prepare(this->_getRandomStartingWordStatement, GET_RANDOM_STARTING_WORD_SQL.c_str(), GET_RANDOM_STARTING_WORD_SQL.length());
  int r5 = mysql_stmt_prepare(this->_getRandomNextWordStatement, GET_RANDOM_NEXT_WORD_SQL.c_str(), GET_RANDOM_NEXT_WORD_SQL.length());

  int r6 = mysql_stmt_prepare(this->_getProperCaseStatement, GET_PROPER_CASE_SQL.c_str(), GET_PROPER_CASE_SQL.length());
  int r7 = mysql_stmt_prepare(this->_getBigramCountStatement, GET_BIGRAM_COUNT_SQL.c_str(), GET_BIGRAM_COUNT_SQL.length());
  int r8 = mysql_stmt_prepare(this->_getPlayerLineCountStatement, GET_PLAYER_LINE_COUNT_SQL.c_str(), GET_PLAYER_LINE_COUNT_SQL.length());
  int r9 = mysql_stmt_prepare(this->_getPlayerNameStatement, GET_PLAYER_NAME_SQL.c_str(), GET_PLAYER_NAME_SQL.length());

  int r10 = mysql_stmt_prepare(this->_insTextRowStatement, INSERT_TEXT_SQL.c_str(), INSERT_TEXT_SQL.length());
  int r11 = mysql_stmt_prepare(this->_getRandomChatMessageStatement, GET_RANDOM_CHAT_MESSAGE_SQL.c_str(), GET_RANDOM_CHAT_MESSAGE_SQL.length());

  if (r1 || r2 || r3 || r4 || r5 || r6 || r7 || r8 || r9 || r10 || r11)
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

    if (this->_getProperCaseStatement)
      mysql_stmt_close(this->_getProperCaseStatement);

    if (this->_getRandomNextWordStatement)
      mysql_stmt_close(this->_getRandomNextWordStatement);

    if (this->_getRandomStartingWordStatement)
      mysql_stmt_close(this->_getRandomStartingWordStatement);

    if (this->_getBigramCountStatement)
      mysql_stmt_close(this->_getBigramCountStatement);

    if (this->_getPlayerLineCountStatement)
      mysql_stmt_close(this->_getPlayerLineCountStatement);

    if (this->_getPlayerNameStatement)
      mysql_stmt_close(this->_getPlayerNameStatement);

    if (this->_insTextRowStatement)
      mysql_stmt_close(this->_insTextRowStatement);

    if (this->_getRandomChatMessageStatement)
      mysql_stmt_close(this->_getRandomChatMessageStatement);
    
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

  int r4 = mysql_query(this->_conn, "CREATE TABLE IF NOT EXISTS markovsbot.fulltext_log "
    "(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, name CHAR(24) NOT NULL, `text` CHAR(255), FULLTEXT text_i(`text`))");

  if (r1 || r2 || r3 || r4)
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

tuple MBConnection::getRandomStartingWord(const std::string& name)
{  
  MYSQL_BIND param[1];
  memset(param, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(param, name);  

  long id;
  char word[32] = {0};  
  MYSQL_BIND result[2];
  memset(result, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(result, id);
  setBindValue<1>(result, word, 32);

  if (mysql_stmt_bind_param(this->_getRandomStartingWordStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getRandomStartingWordStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getRandomStartingWordStatement))
  {
    std::cout << "Executing select from random starting word failed" << std::endl;
    throw 0;
  }
  
  mysql_stmt_store_result(this->_getRandomStartingWordStatement);  
  if (mysql_stmt_fetch(this->_getRandomStartingWordStatement)) 
    return make_tuple();

  return make_tuple(id, word);
}

tuple MBConnection::getRandomNextWord(const std::string& name, long wordId)
{
  MYSQL_BIND param[2];
  memset(param, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(param, name);
  setBindValue<1>(param, wordId);

  long id;
  char word[32] = {0};
  MYSQL_BIND result[2];
  memset(result, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(result, id);
  setBindValue<1>(result, word, 32);

  if (mysql_stmt_bind_param(this->_getRandomNextWordStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getRandomNextWordStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getRandomNextWordStatement))
  {
    std::cout << "Executing select from random next word failed" << mysql_stmt_error(this->_getRandomNextWordStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getRandomNextWordStatement);
  if (mysql_stmt_fetch(this->_getRandomNextWordStatement)) 
    return make_tuple();

  return make_tuple(id, word);
}

std::string MBConnection::getProperCase(const std::string& name)
{
  MYSQL_BIND param[1];
  memset(param, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(param, name);  
  
  char playerName[24] = {0};
  MYSQL_BIND result[1];
  memset(result, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(result, playerName, 24);

  if (mysql_stmt_bind_param(this->_getProperCaseStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getProperCaseStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getProperCaseStatement))
  {
    std::cout << "Executing select from proper case statement failed" << mysql_stmt_error(this->_getProperCaseStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getProperCaseStatement);
  if (mysql_stmt_fetch(this->_getProperCaseStatement)) 
    return "";

  return playerName;
}

TupleList MBConnection::getBigramCount(const std::string& word1, const std::string& word2)
{
  MYSQL_BIND param[2];
  memset(param, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(param, word1);  
  setBindValue<1>(param, word2);
  
  long nameId;
  long lineCount;
  MYSQL_BIND result[2];
  memset(result, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(result, nameId);
  setBindValue<1>(result, lineCount);

  if (mysql_stmt_bind_param(this->_getBigramCountStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getBigramCountStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getBigramCountStatement))
  {
    std::cout << "Executing select from bigram count failed" << mysql_stmt_error(this->_getBigramCountStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getBigramCountStatement);
  TupleList resultSet;

  while (!mysql_stmt_fetch(this->_getBigramCountStatement)) 
    resultSet.push_back(make_tuple(static_cast<int>(nameId), static_cast<int>(lineCount)));

  return resultSet;
}

int MBConnection::getPlayerLineCount(long playerId)
{
  MYSQL_BIND param[1];
  memset(param, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(param, playerId);  
  
  long lineCount;
  MYSQL_BIND result[1];
  memset(result, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(result, lineCount);

  if (mysql_stmt_bind_param(this->_getPlayerLineCountStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getPlayerLineCountStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getPlayerLineCountStatement))
  {
    std::cout << "Executing select from player line count statement failed" << mysql_stmt_error(this->_getPlayerLineCountStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getPlayerLineCountStatement);
  if (mysql_stmt_fetch(this->_getPlayerLineCountStatement)) 
    return 0;

  return lineCount;
}

std::string MBConnection::getPlayerName(long playerId)
{
  MYSQL_BIND param[1];
  memset(param, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(param, playerId);  
  
  char playerName[24] = {0};
  MYSQL_BIND result[1];
  memset(result, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(result, playerName, 24);

  if (mysql_stmt_bind_param(this->_getPlayerNameStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getPlayerNameStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getPlayerNameStatement))
  {
    std::cout << "Executing select from proper case statement failed" << mysql_stmt_error(this->_getPlayerNameStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getPlayerNameStatement);
  if (mysql_stmt_fetch(this->_getPlayerNameStatement)) 
    return "";

  return playerName;
}

void MBConnection::log(const std::string& name, const std::string& message)
{
  MYSQL_BIND param[2];
  memset(param, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(param, name);
  setBindValue<1>(param, message);

  if (mysql_stmt_bind_param(this->_insTextRowStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_insTextRowStatement))
  {
    std::cout << "Executing insert to log table failed" << std::endl;
    throw 0;
  }

  return;
}

std::string MBConnection::getRandomChatMessage(const std::string& substring)
{
  char escaped_cc[255] = {0};
  mysql_real_escape_string(this->_conn, escaped_cc, substring.c_str(), substring.length());

  std::string escaped_substring(escaped_cc);

  MYSQL_BIND param[1];
  memset(param, 0, sizeof(MYSQL_BIND));
  setBindValue<0>(param, escaped_substring);  
  
  char playerName[24] = {0};
  char message[255] = {0};
  MYSQL_BIND result[2];
  memset(result, 0, 2 * sizeof(MYSQL_BIND));
  setBindValue<0>(result, playerName, 24);
  setBindValue<1>(result, message, 255);

  if (mysql_stmt_bind_param(this->_getRandomChatMessageStatement, param))
  {
    std::cout << "Binding parameters to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_bind_result(this->_getRandomChatMessageStatement, result))
  {
    std::cout << "Binding result to prepared statement failed" << std::endl;
    throw 0;
  }

  if (mysql_stmt_execute(this->_getRandomChatMessageStatement))
  {
    std::cout << "Executing select from get random chat message statement failed" << mysql_stmt_error(this->_getRandomChatMessageStatement) << std::endl;
    throw 0;
  }

  mysql_stmt_store_result(this->_getRandomChatMessageStatement);
  if (mysql_stmt_fetch(this->_getRandomChatMessageStatement)) 
    return "";

  return std::string(playerName) + "> " + message;
}