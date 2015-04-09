#ifndef _MB_CONNECTION_HPP_
#define _MB_CONNECTION_HPP_

#include <boost/python/tuple.hpp>
#include <mysql.h>
#include <string>

typedef std::vector<boost::python::tuple> TupleList;

// TODO refactor
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

  boost::python::tuple getRandomStartingWord(const std::string& name);
  boost::python::tuple getRandomNextWord(const std::string& name, long wordId);

  std::string getProperCase(const std::string& name);
  TupleList getBigramCount(const std::string& word1, const std::string& word2);
  int getPlayerLineCount(long playerId);
  std::string getPlayerName(long playerId);

  void log(const std::string& name, const std::string& text);
  std::string getRandomChatMessage(const std::string& substring);

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
  MYSQL_STMT* _getRandomStartingWordStatement;
  MYSQL_STMT* _getRandomNextWordStatement;
  MYSQL_STMT* _getProperCaseStatement;
  MYSQL_STMT* _getBigramCountStatement;
  MYSQL_STMT* _getPlayerLineCountStatement;
  MYSQL_STMT* _getPlayerNameStatement;
  MYSQL_STMT* _insTextRowStatement;
  MYSQL_STMT* _getRandomChatMessageStatement;
};

#endif
