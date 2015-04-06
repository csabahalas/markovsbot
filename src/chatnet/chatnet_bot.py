import chatnet
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

class Bot(object):
  def __init__(self, conn):
    self.connHandler = chatnet.ChatnetConnectionHandler(conn)
    self.connHandler.register_parser(chatnet.create_parser(chatnet.ChatnetMessages.PRIVATE))
    self.conn = conn
    self.commands = []

  def get_conn(self):
    return self.conn

  def addCommand(self, command):
    self.commands.append(command)
    self.connHandler.register_handler(command.get_type(), command.on_command)

  def run(self):
    self.conn.connect()
    self.conn.login("tetris-", "ralphtango")
    self.conn.gotoArena("#test")
    self.connHandler.run()

  def stop(self):
    self.conn.close()

def main():
  try:
    conn = chatnet.ChatnetConnection("142.4.200.80", 5005)
    bot = Bot(conn)
    bot.addCommand(OwnerCommand(bot))
    bot.addCommand(AboutCommand(bot, "Generates sentences and also matches user based on supplied sentence."))
    bot.addCommand(ShutdownCommand(bot, ["nn", "ceiu", "cdb-man", "b.o.x.", "noldec"]))
    bot.addCommand(HelpCommand(bot, "PM or message in team chat: !say name, !whosaid message"))
    bot.run()  
  finally:
    bot.stop()
  
main()
