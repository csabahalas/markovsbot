#ifndef _MYSQL_HELPER_HPP_
#define _MYSQL_HELPER_HPP_

#include <cstring>
#include <mysql.h>

template <class T, int COLUMN_NO>
constexpr static inline void setBindValue(MYSQL_BIND* bind, T& value)
{
  setBindValue(bind, value);
}

template <int COLUMN_NO>
constexpr static inline void setBindValue(MYSQL_BIND* bind, long& value)
{
  bind[COLUMN_NO].buffer_type = MYSQL_TYPE_LONG;
  bind[COLUMN_NO].buffer = &value;
  bind[COLUMN_NO].buffer_length = sizeof(value);
  bind[COLUMN_NO].is_null = 0;
}

template <int COLUMN_NO>
constexpr static inline void setBindValue(MYSQL_BIND* bind, const std::string& value)
{
  bind[COLUMN_NO].buffer_type = MYSQL_TYPE_STRING;
  bind[COLUMN_NO].buffer = const_cast<char*>(value.c_str());
  bind[COLUMN_NO].buffer_length = value.length();
  bind[COLUMN_NO].is_null = 0; 
}

template <int COLUMN_NO>
constexpr static inline void setBindValue(MYSQL_BIND* bind, char& value)
{
  bind[COLUMN_NO].buffer_type = MYSQL_TYPE_TINY;
  bind[COLUMN_NO].buffer = &value;
  bind[COLUMN_NO].buffer_length = sizeof(value);
  bind[COLUMN_NO].is_null = 0;    
}

#endif