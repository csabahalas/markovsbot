import mbdb
import sys

db = mbdb.MBDatabase("*", "*", "127.0.0.1")
c = db.new_connection()
arg_name = sys.argv[1][:10]

try:
  (word_id, word) = c.get_random_starting_word(arg_name)
except:
  c.close()
  
name = c.get_proper_case(arg_name)

for x in range(0, 16):
  try:
    (word_id, word2) = c.get_random_next_word(arg_name, word_id)    
    if word_id == 206158430271:
      break
    word += " %s" % word2
  except:
    break

sentence = ""
if name:
  sentence = "%s> %s" % (name, word)
else:
  sentence = "%s> %s" % (arg_name, word)

c.close()
print sentence

