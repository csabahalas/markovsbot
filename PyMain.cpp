#include <boost/python.hpp>
#include <string>
#include "MBDatabase.hpp"
#include "MBConnection.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(mbdb)
{
  class_<MBConnection>("MBConnection", init<std::string, std::string, std::string>());
  class_<MBDatabase>("MBDatabase", init<std::string, std::string, std::string>())
    .def("new_connection", &MBDatabase::newConnection);    
}