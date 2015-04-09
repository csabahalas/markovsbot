import math
import mbdb
import re
import time

def main():
    FILENAME = "session.log"
    db = mbdb.MBDatabase('*', '*', '127.0.0.1')
    
    c = db.new_connection()    
    f = open(FILENAME)
    lines = f.readlines()

    rgx = re.compile(r"^\s+([^<].+?)> ([^\?].*?)$")
    lengths = []    
    wordsToId = {}
    playersToId = {}
    
    for l in lines:         
        if l[0] != " ":
            continue
        match = rgx.match(l)
        if not match:
            continue

        name = str(match.group(1).decode("ascii", "ignore"))
        blurb = str(match.group(2).decode("ascii", "ignore"))

        c.log_chat(name, blurb)
    
    c.close()      
    
main()
    

