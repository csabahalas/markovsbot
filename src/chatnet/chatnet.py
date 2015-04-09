import re
import select
import socket
import threading
import time

class ChatnetConnection(object):
  def __init__(self, ip, port):
    self.ip = ip
    self.port = port

  def add_disconnect_handler(self, *fn):
    self.disconnectFns = fn

  def notify_disconnect(self):
    if (self.disconnectFns):
      map(lambda fn: fn.on_disconnect(self), self.disconnectFns)

  def send_priv(self, target, message):
    self.send("SEND:PRIV:%s:%s\n" % (target, message))

  def send_freq(self, freq, message):
    self.send("SEND:FREQ:%s:%s\n" % (freq, message))

  def connect(self):
    try:
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((self.ip, self.port))
      self.sock.setblocking(0)
      
      try:
        if self.iotask:
          self.iotask.stop()        
      except:
        pass
      
      self.iotask = ChatnetIoTask(self.sock, self)
      self.iotask.run()
      
    except socket.error, e:
      self.notify_disconnect()

  def close(self):
    self.iotask.stop()
    self.iotask.join()
    self.sock.close()

  def read(self):
    return self.iotask.read()

  def send(self, message):
    self.iotask.send(message)

  def login(self, username, password):
    self.send("LOGIN:1:%s:%s\n" % (username, password))
    time.sleep(4) # make proper handler

  def gotoArena(self, arena):
    self.send("GO:%s\n" % (arena))
  
  def send_cmd(self, cmd):
    self.send("SEND:CMD:%s\n" % cmd)

class ChatnetIoTask(object):
  def __init__(self, sock, conn):
    self.sock = sock
    self.conn = conn
    self.running = False
    self.sendNotEmpty = threading.Condition()
    self.recvNotEmpty = threading.Condition()
    self.recvBufferLock = threading.Lock()
    self.sendStr = ""
    self.recvBufStr = ""

  def read(self):
    try:
      self.recvNotEmpty.acquire()
      while not self.recvBufStr:
        self.recvNotEmpty.wait(5)
        if not self.running:          
          return []
        
    finally:
      self.recvNotEmpty.release()
    
    message = ""
    messages = []
    try:
      self.recvBufferLock.acquire()

      i = 0
      for char in self.recvBufStr:
        if char == '\n':
          messages.append(message)
          message = ""
          self.recvBufStr = self.recvBufStr[i:]
          i = 0
        else:
          message += char
        i += 1
      self.recvBufStr = ""    
    finally:
      self.recvBufferLock.release()

    return messages

  def join(self):
    self.sendTh.join()
    self.readTh.join()

  def stop(self):
    self.running = False

  def do_read(self):
    self.running = True
    while self.running:
      ready = select.select([self.sock], [], [], 10)
      if not ready[0]:
        continue
      
      data = self.sock.recv(4096)      
      if not data:
        self.conn.notify_disconnect()
        return

      try:
        self.recvBufferLock.acquire()
        self.recvBufStr += data
      finally:
        self.recvBufferLock.release()
      
      try:
        self.recvNotEmpty.acquire()
        self.recvNotEmpty.notify()
      finally:
        self.recvNotEmpty.release()

  def do_send(self):
    try:
      self.running = True
      self.sendNotEmpty.acquire()
      delay = 0.3
      lastSend = time.time()
      
      while self.running:
        while not self.sendStr:
          self.sendNotEmpty.wait(10)
          if not self.running:
            return

        if time.time() - lastSend > 4:
          delay = 0.3
        
        self.sock.sendall(self.sendStr)
        lastSend = time.time()
        time.sleep(delay)
        delay *= 1.5
        
        if delay > 4:
          delay = 0.3
        self.sendStr = ""
    finally:
      self.sendNotEmpty.release()
      
  def run(self):
    self.readTh = threading.Thread(target=self.do_read)
    self.sendTh = threading.Thread(target=self.do_send)
    self.readTh.start()
    self.sendTh.start()
  
  def send(self, message):
    try:
      self.sendNotEmpty.acquire()
      self.sendStr += message
      self.sendNotEmpty.notify()
    finally:
      self.sendNotEmpty.release()

class ChatnetMessages:
  PRIVATE = 0
  FREQ = 1
  UNKNOWN = 2

  @staticmethod
  def type_of(message):
    if message.startswith("MSG:PR"):
      return ChatnetMessages.PRIVATE
    elif message.startswith("MSG:FR"):
      return ChatnetMessages.FREQ
    return ChatnetMessages.UNKNOWN

class ChatnetMessageParser(object):
  def __init__(self, parser_type, regExprStr):
    self.parser_type = parser_type
    self.pattern = re.compile(regExprStr)

  def parse(self, message):
    m = self.pattern.match(message)
    if not m:
      raise Exception("Message string doesn't follow this parser pattern.")
    return m

class ChatnetConnectionHandler(object):
  def __init__(self, conn):
    self.conn = conn
    self.typesToHandlers = {}
    self.parsers = []

  def register_handler(self, messageType, fn):
    try:
      self.typesToHandlers[messageType].append(fn)
    except:
      self.typesToHandlers[messageType] = []
      self.typesToHandlers[messageType].append(fn)

  def register_parser(self, parser):
    self.parsers.append(parser)

  def run(self):
    while 1:
      messages = self.conn.read()      
      for message in messages:
        for parser in self.parsers:
          try:
            map(lambda handler: handler(parser.parse(message)),
                self.typesToHandlers[ChatnetMessages.type_of(message)])
          except Exception, e:
            continue

def printing_handler(data):
  data = data.groups()
  print reduce(lambda x, y: x + " " + y, data)

def create_parser(pType):
  if pType == ChatnetMessages.PRIVATE:
    return ChatnetMessageParser(ChatnetMessages.PRIVATE, "^MSG:PRIV:(?P<name>.+?):(?P<message>.+)$")
  elif pType == ChatnetMessages.FREQ:
    return ChatnetMessageParser(ChatnetMessages.FREQ, "^MSG:FREQ:(?P<name>.+?):(?P<message>.+)$")
