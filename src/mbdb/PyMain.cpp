#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <string>
#include "MBDatabase.hpp"
#include "MBConnection.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(mbdb)
{
  class_<TupleList>("TupleList")
    .def(vector_indexing_suite<TupleList, true>());

  class_<MBConnection>("MBConnection", init<std::string, std::string, std::string>())
    .def("insert_player_row", &MBConnection::insertPlayerRow)
    .def("insert_word_row", &MBConnection::insertWordRow)
    .def("insert_conv_row", &MBConnection::insertConvRow)
    .def("get_random_starting_word", &MBConnection::getRandomStartingWord)
    .def("get_random_next_word", &MBConnection::getRandomNextWord)
    .def("get_proper_case", &MBConnection::getProperCase)
    .def("get_bigram_count", &MBConnection::getBigramCount)
    .def("get_player_line_count", &MBConnection::getPlayerLineCount)
    .def("get_player_name", &MBConnection::getPlayerName)
    .def("close", &MBConnection::close);

  class_<MBDatabase>("MBDatabase", init<std::string, std::string, std::string>())
    .def("new_connection", &MBDatabase::newConnection);        
}