import mbdb
import sys

db = mbdb.MBDatabase("*", "*", "127.0.0.1")
c = db.new_connection()
t = c.get_rand_message(" ".join(sys.argv[1:]))
print t


c.close()
