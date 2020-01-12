# Implementation of the parser of "citatepedia"
import html
import urllib.request
import sys
import string
from ast import literal_eval
from unidecode import unidecode
from pyxform.utils import unichr

# TODO: Are there any non-romanian characters?
def applyDiacritics(text):
  # Transform the slash errors from utf-8 decode into diacritics
  
  # Small diacritics
  text = text.replace("\\xe2", "â")
  text = text.replace("\\xee", "î")
  text = text.replace("\\xe3", "ă")
  text = text.replace("\\xfe", "ţ")
  text = text.replace("\\xba", "ş")

  # Big diacritics
  text = text.replace("\\xc3", "Ă")
  text = text.replace("\\xc2", "Â")
  text = text.replace("\\xaa", "Ş")
  text = text.replace("\\xce", "Î")
  text = text.replace("\\xde", "Ţ")
  
  # Other characters (only for 2-bytes code points)
  repl = {}
  index = 0
  limit = len(text)
  while index != limit - 3:
    if (text[index] == '\\') and (text[index + 1] == 'x'):
      curr = text[index : (index + 4)]
      repl[curr] = unidecode(unichr(ord(literal_eval("b'{}'".format(curr)))))
      index += 3
    index += 1
  for prev in repl.keys():
    text = text.replace(prev, repl[prev])
  return text

def getHtmlText(url):
# get the html text from this url
  # Connect to the site
  request = urllib.request.Request(url)
  response = urllib.request.urlopen(request)
    
  # Decode the site into utf-8
  text = response.read().decode('utf-8', errors = 'backslashreplace')
  
  # The utf-8 decoded file may have some problems. First solve them.
  text = applyDiacritics(text)
  return text

def computeNumberOfPages(poet):
  initialUrl = "http://poezii.citatepedia.ro/de.php?a=" + poet
  text = getHtmlText(initialUrl)
  pagePattern = "<option value=\"p="
  posLastPage = text.rfind(pagePattern)
  
  # If the pattern is not found, there is only one page
  noPages = 1
  if posLastPage != -1:
    offset = posLastPage + len(pagePattern)
    noPages = int(text[offset:(offset + text[offset:].find("&"))])
  return noPages
  
def extractLinksFromPage(poet, page):
  pageUrl = "http://poezii.citatepedia.ro/de.php?a=" + poet + "&p=" + str(page)
  text = getHtmlText(pageUrl)
  pattern = "data-url=\""
  
  links = []
  pos = text.find(pattern)
  while pos != -1:
    offset = pos + len(pattern)
    link = text[offset:(offset + text[offset:].find('"'))]
    
    # Get only the links with ids
    if link.find("id=") != -1:
      links.append(link)
    text = text[offset:]
    pos = text.find(pattern)
  return links

def parsePoetry(line):
  # How many opened "<" are currenly in the text
  countWriters = 0
  poetry = ""
  for c in line:
    # Start a writer
    if c == '<':
      countWriters += 1
      poetry += " "
    elif c == '>':
      # Check if have many more unclosed writers
      if countWriters != 0:
        countWriters -= 1
    # No writers? Then we have a free text, which should be part of the poetry
    elif countWriters == 0:
      poetry += c
  return poetry

def parsePoemFromLink(link):
  text = getHtmlText(link)
  lines = text.splitlines()
  
  poem = ""
  beginContentPattern = "<div class=\"q\">"
  endContentPattern = "<p class=pa><a href="
  for line in lines:
    if line.find(beginContentPattern) != -1:
      endPos = line.find(endContentPattern)
      assert endPos != -1
      poem = parsePoetry(line[:endPos])
      break
  assert poem != ""
  return poem

def extractLinks(poet):
  noPages = computeNumberOfPages(poet)
  links = []
  print(poet + " has " + str(noPages) + " page(s)") 
  for page in range(1, noPages + 1):
    links += extractLinksFromPage(poet, page)
  return links
    
def parsePoet(poet):
  output = open("poets/poetry/" + poet + "_poems.txt", "w")
  links = extractLinks(poet)
  for link in links:
    poem = parsePoemFromLink(link)
    output.write(poem + '\n')
  pass
    
def searchPoetRefs(text):
  pattern = "de.php?a="
  lenPattern = len(pattern)
  poetRefs = []
  lines = text.splitlines()
  for line in lines:
    curr = line
    pos = curr.find(pattern)
    while pos != -1:
      posQuote = pos + lenPattern
      while posQuote < len(curr) and curr[posQuote] != '"':
        posQuote += 1
      poet = curr[(pos + lenPattern):posQuote]
      poetRefs.append(poet)
      # Continue the search on this line
      curr = curr[posQuote:]
      pos = curr.find(pattern)
  return poetRefs
    
def extractPoetsWithLetter(capitalLetter):
  url = "http://poezii.citatepedia.ro/autori.php?c=" + capitalLetter
  text = getHtmlText(url)
  return searchPoetRefs(text)
  
def extractAllPoetRefs():
  output = open("poets/list_poets.txt", "w")
  count = 0
  for capitalLetter in string.ascii_uppercase:
    print("Extracting poets with letter: " + capitalLetter)
    poetRefs = extractPoetsWithLetter(capitalLetter)
    for ref in poetRefs:
      output.write(ref + '\n')
      count += 1
  print("Done! Extracted " + str(count) + " poets")
  output.close()
  return

def main():
  # parsePoet("A.C.+Dragodan")
  # parsePoet("Mihai+Eminescu")
  print(parsePoemFromLink("http://www.citatepedia.ro/index.php?id=328036"))
 
if __name__ == '__main__':
    main()
