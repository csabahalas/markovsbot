from __future__ import division
import math
import re
import sqlite3

'''
Python 2.7
Small script to convert ss convo logs to sqlite3 sentence generator database
'''

def main():
    FILENAME = "session.log"
    conn = sqlite3.connect("connos.db")
    c = conn.cursor()
    c.execute("CREATE TABLE IF NOT EXISTS player_names (id INTEGER PRIMARY KEY, name CHAR(64) NOT NULL UNIQUE)")
    c.execute("CREATE TABLE IF NOT EXISTS words (id INTEGER PRIMARY KEY, word CHAR(32) NOT NULL UNIQUE)")
    # TODO foreign keys unavailable?
    c.execute("CREATE TABLE IF NOT EXISTS conversations (name_id INTEGER NOT NULL, word_a INTEGER NOT NULL, word_b INTEGER NOT NULL, is_starting INTEGER NOT NULL)")
    c.execute("CREATE INDEX name_p_i ON player_names(name)")
    c.execute("CREATE INDEX word_i ON words(word)")
    c.execute("CREATE INDEX name_i ON conversations(name_id)")
    c.execute("CREATE INDEX wa_i ON conversations(word_a)")
    c.execute("CREATE INDEX wb_i ON conversations(word_b)")    
    
    conn.commit()    

    f = open(FILENAME)
    lines = f.readlines()

    rgx = re.compile(r"^\s+([^<].+?)> ([^\?].*?)$")
    index = 0
    lengths = []
    
    for l in lines:
        if l[0] != " ":
            continue
        match = rgx.match(l)
        if not match:
            continue

        name = match.group(1).decode("ascii", "ignore")
        blurb = match.group(2).decode("ascii", "ignore")

        arr_words = blurb.split(' ')

        c.execute("SELECT id FROM player_names WHERE name=?", (name,))
        row = c.fetchone()
        if not row:
          c.execute("INSERT INTO player_names (name) VALUES (?)", (name,))            
        c.execute("SELECT id FROM player_names WHERE name=?", (name,))
        player_id = c.fetchone()[0]       
        
        starting = 1
        for i_w in range(len(arr_words) - 1):
            word_a = arr_words[i_w]
            word_b = ""
            if i_w < len(arr_words) - 1:
                word_b = arr_words[i_w + 1]

            if len(word_a) > 32 or len(word_b) > 32:
              continue
                
            c.execute("SELECT id FROM words WHERE word=?", (word_a,))
            row = c.fetchone()
            if not row:
              c.execute("INSERT INTO words (word) VALUES (?)", (word_a,))
            c.execute("SELECT id FROM words WHERE word=?", (word_a,))  
            word_a_id = c.fetchone()[0]

            c.execute("SELECT id FROM words WHERE word=?", (word_b,))
            row = c.fetchone()
            if not row:
              c.execute("INSERT INTO words (word) VALUES (?)", (word_b,))
            c.execute("SELECT id FROM words WHERE word=?", (word_b,))  
            word_b_id = c.fetchone()[0]
            c.execute("INSERT INTO conversations (name_id, word_a, word_b, is_starting) VALUES (?,?,?,?)", (player_id, word_a_id, word_b_id, starting))
                        
            if index == 200000:                
                conn.commit()
                index = 0
            starting = 0
            index += 1
    
    conn.commit()
    conn.close()
        
    
main()
    
