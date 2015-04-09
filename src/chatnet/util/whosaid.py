from __future__ import division
import mbdb
import operator
import sys

db = mbdb.MBDatabase('*', '*', "127.0.0.1")
c = db.new_connection()

player_data = {}
player_confidence = {}
current_total_bigram_count = 0
words = sys.argv[1:]
first = True
for i in range(0, len(words)):
  word_a = words[i]
  word_b = ""
  if i + 1 < len(words):
    word_b = words[i + 1]
  elif not first:
    break
  
  bigram_data = c.get_bigram_count(word_a, word_b)
  first = False

  tot_count = 0
  tot_bigram_count = 0
  
  for (k, v) in bigram_data:
    tot_count += c.get_player_line_count(k)
    tot_bigram_count += v

  current_total_bigram_count += tot_bigram_count

  for (player_id, n_lines) in bigram_data:
    line_count = c.get_player_line_count(player_id)
    try:
      player_data[player_id] *= (n_lines * 1000 / line_count) * (n_lines * 1000 / tot_count) * (n_lines * 100 / tot_bigram_count)
      player_confidence[player_id] = (player_confidence[player_id][0] + n_lines, player_confidence[player_id][1] + tot_bigram_count)
    except KeyError:
      player_data[player_id] = (n_lines * 1000 / line_count) * (n_lines * 1000 / tot_count) * (n_lines * 100 / tot_bigram_count)
      player_confidence[player_id] = (n_lines, current_total_bigram_count)
  
for player_id in player_data:
  player_data[player_id] *= c.get_player_line_count(player_id)

print player_data

try:
  max_player_id = max(player_data.iteritems(), key=operator.itemgetter(1))[0]
  max_name = c.get_player_name(max_player_id)
  max_player_confidence = player_confidence[max_player_id][0] / player_confidence[max_player_id][1]
  print "%s said %s (%s%% confidence)" % (max_name, " ".join(sys.argv[1:]), 100 * round(max_player_confidence, 4))
finally:
  c.close()

