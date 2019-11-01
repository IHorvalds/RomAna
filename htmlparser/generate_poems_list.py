# The role of this script:
# Uses the site romanianvoice.com to take all the poem titles of all poets written in "poets.txt"
import urllib.request
import html
import sys
import extract_poems

def main():
    f = open("poets/poets.txt")
    for line in f:
        poet = line.strip()
        extract_poems.savePoems(poet, "poets/poems/" + poet + "_list_poems.txt")
        
if __name__ == '__main__':
    main()
