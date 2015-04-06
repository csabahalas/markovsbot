import math
import mbdb
import re
import time

'''
Python 2.7
Small script to convert ss convo logs to sqlite3 sentence generator database
'''

def main():
    FILENAME = "session.log"
    db = mbdb.MBDatabase('*', '*', '127.0.0.1')
    db.create_schema()
    
    c = db.new_connection()    
    f = open(FILENAME)
    lines = f.readlines()

    rgx = re.compile(r"^\s+([^<].+?)> ([^\?].*?)$")
    lengths = []    
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
            c.insert_player_row(player_id, name)
        
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
                c.insert_word_row(word_a_id, word_a)
            
            word_b_id = current_word_id
            try:
                word_b_id = wordsToId[word_b]
            except KeyError:                
                current_word_id += 1
                wordsToId[word_b] = word_b_id
                c.insert_word_row(word_b_id, word_b)

            c.insert_conv_row(player_id, word_a_id, word_b_id, starting)
            starting = 0              
    
    c.close()      
    
main()
    
