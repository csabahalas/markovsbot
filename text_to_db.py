import math
import mysql.connector
import re
import time

'''
Python 2.7
Small script to convert ss convo logs to sqlite3 sentence generator database
'''

def main():
    FILENAME = "session.log"
    conn = mysql.connector.connect(user='root', password='*',
        host='127.0.0.1')
    c = conn.cursor(prepared=True)
    c.execute("CREATE DATABASE IF NOT EXISTS markovsbot")
    # c.execute("USE markovsbot") not supported...
    
    c.execute("CREATE TABLE IF NOT EXISTS markovsbot.player_names (id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, name CHAR(24) NOT NULL, INDEX name_i (name)) ENGINE=INNODB")
    c.execute("CREATE TABLE IF NOT EXISTS markovsbot.words (id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, word CHAR(32) NOT NULL, INDEX word_i (word)) ENGINE=INNODB")

    c.execute("CREATE TABLE IF NOT EXISTS markovsbot.conversations (id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, name_id INT NOT NULL, word_a INT NOT NULL, word_b INT NOT NULL, is_starting TINYINT NOT NULL, "
        "FOREIGN KEY word_a_fk (word_a) REFERENCES words (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
        "FOREIGN KEY word_b_fk (word_b) REFERENCES words (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
        "FOREIGN KEY name_fk (name_id) REFERENCES player_names (id) ON DELETE CASCADE ON UPDATE RESTRICT,"
        "INDEX name_i (name_id), INDEX wa_i (word_a)) ENGINE=INNODB")

    f = open(FILENAME)
    lines = f.readlines()

    rgx = re.compile(r"^\s+([^<].+?)> ([^\?].*?)$")
    lengths = []
    index = 0
    n_statements_per_commit = 2
    wordsToId = {}
    playersToId = {}
    current_player_id = 1
    current_word_id = 1

    for l in lines:         
        if l[0] != " ":
            continue
        match = rgx.match(l)
        if not match:
            continue

        name = str(match.group(1).decode("ascii", "ignore"))
        blurb = str(match.group(2).decode("ascii", "ignore"))

        arr_words = blurb.split(' ')        
                
        player_id = current_player_id 
        try:
            player_id = playersToId[name]
        except KeyError:
            current_player_id += 1            
            playersToId[name] = player_id            
            c.execute("INSERT INTO markovsbot.player_names (id,name) VALUES (?,?)", (player_id, name))                
        
        starting = 1
        for i_w in range(len(arr_words) - 1):
            word_a = arr_words[i_w]
            word_b = ""
            if i_w < len(arr_words) - 1:
                word_b = arr_words[i_w + 1]

            if len(word_a) > 32 or len(word_b) > 32:
              continue                
                        

            word_a_id = current_word_id
            try:
                word_a_id = wordsToId[word_a]
            except KeyError:                
                current_word_id += 1
                wordsToId[word_a] = word_a_id
                c.execute("INSERT INTO markovsbot.words (id,word) VALUES (?,?)", (word_a_id, word_a))            
            
            word_b_id = current_word_id
            try:
                word_b_id = wordsToId[word_b]
            except KeyError:                
                current_word_id += 1
                wordsToId[word_b] = word_b_id
                c.execute("INSERT INTO markovsbot.words (id,word) VALUES (?,?)", (word_b_id, word_b))                        

            c.execute("INSERT INTO markovsbot.conversations (name_id, word_a, word_b, is_starting) VALUES (?,?,?,?)", (player_id, word_a_id, word_b_id, starting))
            starting = 0

            if n_statements_per_commit == index:
                index = 0
                conn.commit()
            index += 1                            
    
    conn.commit()
    c.close()
    conn.close()
        
    
main()
    
