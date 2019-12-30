# The role of this script:
# Uses the site poetii-nostri.ro to take all the poets from the website
import collections
import urllib.request
import html
import sys

def erase(text, begin, end):
    # Erase the portion of text, which is between "begin" and "end"
    posBegin = text.find(begin)
    if posBegin != -1:
      posEnd = text.find(end, posBegin)
      toReplace = text[posBegin:posEnd]
      return text.replace(toReplace, "")
    else:
      return text

def keepPoetRefs(text):
# From the html page, keep only the lines with references to poets
  keepedLines = []
  lines = text.splitlines()
  for line in lines:
    autorPos = line.find("-autor-")
    if autorPos != -1:
      titlePos = line[autorPos:].find("title=")
      if titlePos != -1:
        keepedLines.append(line[:(titlePos + autorPos - 1)])
  return keepedLines

def keepPoets(poetRefs):
# From the poet references parse the poets
  dictPoets = {}
  for ref in poetRefs:
    autorPos = ref.find("-autor-")
    openSlashPos = autorPos
    while openSlashPos != 0 and ref[openSlashPos - 1] != '/':
      openSlashPos -= 1
    closedSlashPos = autorPos
    while closedSlashPos != len(ref) and ref[closedSlashPos] != '/':
      closedSlashPos += 1
    poetId = 0
    current = closedSlashPos
    while ord(ref[current - 1]) >= ord('0') and ord(ref[current - 1]) <= ord('9'):
      poetId = poetId * 10 + int(ref[current - 1])
      current -= 1
    poetId = int(str(poetId)[::-1])
    dictPoets[poetId] = ref[openSlashPos:closedSlashPos]
  
  # And order by the poetId
  dictPoets = collections.OrderedDict(sorted(dictPoets.items()))
  sorted_ = []
  for poetId, ref in dictPoets.items():
    sorted_.append([poetId, ref])
  return sorted_

def getHtmlText(url):
# get the html text from this url
  # Connect to the site
  request = urllib.request.Request(url)
  response = urllib.request.urlopen(request)
    
  # Decode the site into utf-8
  text = response.read().decode('utf-8')
  return text
   
def extractTypeOfPoets(poetType):
# extract the list of poets of type "poetType"
  url = "http://poetii-nostri.ro/poeti-" + poetType + "i/"
  
  text = getHtmlText(url)
  poetRefs = keepPoetRefs(text)
  poets = keepPoets(poetRefs)
  return poets
  
def extractClassicalPoets():
# extract the classical poets
  return extractTypeOfPoets("clasic")
    
def extractContemporaryPoets():
# extract the contemporary poets
  return extractTypeOfPoets("contemporan")
    
def extractForeignPoets():
# extract the foreign poets
  return extractTypeOfPoets("strain")
    
def extractAllPoets(useClassical, useContemporary, useForeign):
  classical = []
  if useClassical:
    classical = extractClassicalPoets()
  
  contemporary = []
  if useContemporary:
    contemporary = extractContemporaryPoets()
 
  foreign = []
  if useForeign:
    foreign = extractForeignPoets()
    
  all_ = classical + contemporary + foreign
  output = open("poets/list_poets.txt", "w")
  for elem in all_:
    output.write(elem[1] + '\n')
  output.close()
  return

def main():
  if len(sys.argv) < 2:
    print("Usage: python3 " + sys.argv[0] + " [type of poet]")
    sys.exit(1)
  extractAllPoets(True, True, False)
  
if __name__ == '__main__':
    main()
