# Implementation of the parser of "citatepedia"
import html
import urllib.request
import sys
import string
from ast import literal_eval
from unidecode import unidecode
from pyxform.utils import unichr

def applyDiacritics(text):
# Transform the slash errors from utf-8 decode into diacritics or variants  
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
  
  # Replace other characters (only for 2-bytes code points)
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
# Compute how many pages in the website poet has
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
# Extract the links from the current page of poet 
  pageUrl = "http://poezii.citatepedia.ro/de.php?a=" + poet + "&p=" + str(page)
  text = getHtmlText(pageUrl)
  pattern = "data-url=\""
  
  links = []
  pos = text.find(pattern)
  while pos != -1:
    # Extract the link
    offset = pos + len(pattern)
    link = text[offset:(offset + text[offset:].find('"'))]
    
    # Get only the links with ids
    if link.find("id=") != -1:
      links.append(link)
    text = text[offset:]
    pos = text.find(pattern)
  return links

def parsePoetry(line):
# Parse the poetry from the current line, by eliminating the rest of html code
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
# Parse the poem from the current link
  text = getHtmlText(link)
  lines = text.splitlines()
  
  poem = ""
  # Pattern with which the content begins
  beginContentPattern = "<div class=\"q\">"
  # Pattern with which the poetry may begin (for those poems where there is no title)
  beginPoetryPattern = "</h3>"
  # Pattern with which the content ends
  endContentPattern = "<p class=pa><a href="
  for line in lines:
    posContent = line.find(beginContentPattern)
    # Have we found the line with the content?
    if posContent != -1:
      # Any title?
      beginPos = line.find(beginPoetryPattern)
      if beginPos == -1:
        beginPos = posContent
      endPos = line.find(endContentPattern)
      assert endPos != -1
      # Return the poem without its title (if any)
      poem = parsePoetry(line[beginPos : endPos])
      break
  assert poem != ""
  return poem

def extractLinks(poet):
# Extract the links to the poems of the current poet
  noPages = computeNumberOfPages(poet)
  links = []
  print(poet + " has " + str(noPages) + " page(s)") 
  for page in range(1, noPages + 1):
    links += extractLinksFromPage(poet, page)
  return links
    
def parsePoet(poet):
# Parse all the poetry of the current poet, if he has any links to his poems
  links = extractLinks(poet)
  hadLinks = len(links) != 0
  if hadLinks:
    output = open("poets/poetry/" + poet + "_poems.txt", "w")
    for link in links:
      poem = parsePoemFromLink(link)
      output.write(poem + '\n')
  return hadLinks
    
def parseAllPoets(firstLine = 1):
# Parse the poetry of each of the poets present in the file
# Start with firstLine of the file
  # Compute how big the list is
  with open('poets/list_poets.txt') as input:
    countLines = sum(1 for _ in input)
  input = open("poets/list_poets.txt", "r")
  
  # If a poet did not have any poems, update the list, by removing his/her name
  nonEmptyPoets = []
  line = 1
  for poet in input:
    if line >= firstLine:
      print("Parse " + poet.strip() + ": progess=[" + "{0:.2f}".format(float(line / countLines) * 100) + "%]")  
      if parsePoet(poet.strip()):
        nonEmptyPoets.append(poet)
    line += 1

  # And update the list
  output = open("poets/list_poets.txt", "w")
  for poet in nonEmptyPoets:
    output.write(poet)
  pass
    
def searchPoetRefs(text):
# Return the references to poets from the current html text
  pattern = "de.php?a="
  lenPattern = len(pattern)
  poetRefs = []
  lines = text.splitlines()
  for line in lines:
    curr = line
    
    # Search for a reference to a poet
    pos = curr.find(pattern)
    while pos != -1:
      # And take only the portion with his/her name
      posQuote = pos + lenPattern
      while posQuote < len(curr) and curr[posQuote] != '"':
        posQuote += 1
      poet = curr[(pos + lenPattern):posQuote]
      poetRefs.append(poet)

      # Continue to search on this line
      curr = curr[posQuote:]
      pos = curr.find(pattern)
  return poetRefs
    
def extractPoetsWithLetter(capitalLetter):
# Extract the names of poets which begin with capitalLetter
  url = "http://poezii.citatepedia.ro/autori.php?c=" + capitalLetter
  return searchPoetRefs(getHtmlText(url))
  
def extractAllPoetRefs():
# Extract the names of poets and write them into file
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
  # Generate the list of poets
  extractAllPoetRefs()
  # Parse the poems of each poet
  # parseAllPoets(4050)
  pass
  
if __name__ == '__main__':
    main()
