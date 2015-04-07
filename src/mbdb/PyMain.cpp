#include <boost/python.hpp>
#include <string>
#include "MBDatabase.hpp"
#include "MBConnection.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(mbdb)
{
  class_<MBConnection>("MBConnection", init<std::string, std::string, std::string>())
    .def("insert_player_row", &MBConnection::insertPlayerRow)
    .def("insert_word_row", &MBConnection::insertWordRow)
    .def("insert_conv_row", &MBConnection::insertConvRow)
    .def("get_random_starting_word", &MBConnection::getRandomStartingWord)
    .def("get_random_next_word", &MBConnection::getRandomNextWord)
    .def("get_proper_case", &MBConnection::getProperCase)
    .def("close", &MBConnection::close);

  class_<MBDatabase>("MBDatabase", init<std::string, std::string, std::string>())
    .def("new_connection", &MBDatabase::newConnection);        
}