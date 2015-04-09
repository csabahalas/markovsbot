from __future__ import division
import chatnet
import mbdb
import operator
import re
import time

class Command(object):
  def __init__(self, name, nArgs, bot, type_):
    regex = "^\s*?!%s" % name
    for a in range(0, nArgs):
      regex += "\s+?(.+?)"
    regex += "$"

    self.pattern = re.compile(regex, re.IGNORECASE)
    self.type = type_
    self.bot = bot

  def get_type(self):
    return self.type

  def on_command(self, data):
    m = self.pattern.match(data.groupdict()["message"])
    if m:
      self.run_command(data.groupdict(), m.groups())

class OwnerCommand(Command):
  def __init__(self, bot):
    Command.__init__(self, "owner", 0, bot, chatnet.ChatnetMessages.PRIVATE)

  def run_command(self, data, args):
    self.bot.get_conn().send_priv(data["name"], "nn")

class HelpCommand(Command):
  def __init__(self, bot, helpText):
    Command.__init__(self, "help", 0, bot, chatnet.ChatnetMessages.PRIVATE)
    self.helpText = helpText

  def run_command(self, data, args):
    self.bot.get_conn().send_priv(data["name"], self.helpText)

class AboutCommand(Command):
  def __init__(self, bot, aboutText):
    Command.__init__(self, "about", 0, bot, chatnet.ChatnetMessages.PRIVATE)
    self.aboutText = aboutText

  def run_command(self, data, args):
    self.bot.get_conn().send_priv(data["name"], self.aboutText)

class ShutdownCommand(Command):
  def __init__(self, bot, mods):
    Command.__init__(self, "shutdown", 0, bot, chatnet.ChatnetMessages.PRIVATE)
    self.mods = mods
  
  def run_command(self, data, args):
    for mod in self.mods:
      if mod == data["name"].lower():        
        self.bot.get_conn().send_priv(data["name"], "Shutting down.")
        time.sleep(4)
        self.bot.stop()
        return
    self.bot.get_conn().send_priv(data["name"], "ill nevr leave u bb <3")

class SayCommand(Command):
  def __init__(self, bot):
    Command.__init__(self, "say", 1, bot, chatnet.ChatnetMessages.FREQ)    
  
  def run_command(self, data, args):    
    db = mbdb.MBDatabase("*", "*", "127.0.0.1")    
    c = db.new_connection()
    arg_name = args[0][:10]            
    
    try:
      (word_id, word) = c.get_random_starting_word(arg_name)
    except:
      c.close()
      return
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
    self.bot.get_conn().send_freq("1337", sentence)

class WhoSaidCommand(Command):
  def __init__(self, bot):
    Command.__init__(self, "whosaid", 1, bot, chatnet.ChatnetMessages.FREQ)    
  
  def run_command(self, data, args):
    db = mbdb.MBDatabase("*", "*", "127.0.0.1")
    c = db.new_connection()

    player_data = {}
    player_confidence = {}
    current_total_bigram_count = 0
    words = args[0].split(" ")
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

    try:
      max_player_id = max(player_data.iteritems(), key=operator.itemgetter(1))[0]
      max_name = c.get_player_name(max_player_id)
      max_player_confidence = player_confidence[max_player_id][0] / player_confidence[max_player_id][1]
      self.bot.get_conn().send_freq("1337", "%s said %s (%s%% confidence)" % (max_name, args[0], 100 * round(max_player_confidence, 4)))
      print "%s said %s (%s%% confidence)" % (max_name, args[0], 100 * round(max_player_confidence, 4))
    except Exception, e:
      print e
    finally:
      c.close()

class LookupCommand(Command):
  def __init__(self, bot):
    Command.__init__(self, "lookup", 1, bot, chatnet.ChatnetMessages.FREQ)    
  
  def run_command(self, data, args):
    db = mbdb.MBDatabase("*", "*", "127.0.0.1")
    c = db.new_connection()

    try:
      message = c.get_rand_message(args[0])
      if message:
        self.bot.get_conn().send_freq("1337", message)
    except Exception, e:
      print e
    finally:
      c.close()
   

class Bot(object):
  def __init__(self, conn):
    self.connHandler = chatnet.ChatnetConnectionHandler(conn)
    self.connHandler.register_parser(chatnet.create_parser(chatnet.ChatnetMessages.PRIVATE))
    self.connHandler.register_parser(chatnet.create_parser(chatnet.ChatnetMessages.FREQ))
    self.conn = conn
    self.commands = []

  def get_conn(self):
    return self.conn

  def addCommand(self, command):
    self.commands.append(command)
    self.connHandler.register_handler(command.get_type(), command.on_command)

  def run(self):
    self.conn.connect()
    self.conn.login("UB-Dr Brain", "ralphtango")
    self.conn.gotoArena("")    
    self.connHandler.run()

  def stop(self):
    self.conn.close()

def main():
  try:
    conn = chatnet.ChatnetConnection("142.4.200.80", 5005)
    bot = Bot(conn)
    bot.addCommand(OwnerCommand(bot))
    bot.addCommand(AboutCommand(bot, "Hyperspace is not a democracy."))
    bot.addCommand(ShutdownCommand(bot, ["nn", "ceiu", "cdb-man", "b.o.x.", "noldec"]))
    bot.addCommand(HelpCommand(bot, "In team chat: !say name, !whosaid message, !lookup message"))
    bot.addCommand(SayCommand(bot))
    bot.addCommand(WhoSaidCommand(bot))
    bot.addCommand(LookupCommand(bot))
    bot.run()      
  finally:
    bot.stop()
  
main()
