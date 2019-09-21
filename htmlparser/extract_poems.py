# The role of this script:
# Uses the site romanianvoice.com to take all the poem titles of the poet from the input
import urllib.request
import html
import sys

def keepPoemRefs(text):
# From the html page, keep only the lines with references to his poems
    keepedLines = []
    lines = text.splitlines()
    for line in lines:
        if line.find("<a href=\"../poezii/") != -1:
            keepedLines.append(line)
    return keepedLines

def getPoemTitles(refLines):
# From the page of a poet, get all the titles of his work
    poemTitles = []
    for ref in refLines:
        pos = ref.find(".php")
        if pos == -1:
            print("We got a problem at parsing the poem reference")
            sys.exit(1)
        else:
            
            ref = ref[:pos]
            lastOcc = ref.rfind("/")
            ref = ref[(lastOcc + 1):]
            poemTitles.append(ref)
    return poemTitles

def readHtml(poet):
# Get the html utf-8 version
    # Create the url
    url = "http://www.romanianvoice.com/poezii/poeti/" + poet + ".php"

    # Connect to the site
    request = urllib.request.Request(url)
    response = urllib.request.urlopen(request)
    
    # Decode the site into utf-8
    text = response.read().decode('utf-8', errors = 'backslashreplace')
    return text

def writePoemTitles(poemTitles, fileName):
# Write the poem titles of the current poet in "fileName"
    f = open(fileName, "w")
    for title in poemTitles:
        f.write(title + '\n')
    pass

def savePoems(poet, fileName):
# Save the titles of the poems of "poet" in "fileName"
    text = readHtml(poet)
    writePoemTitles(getPoemTitles(keepPoemRefs(text)), fileName)
    pass

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 " + sys.argv[0] + " [poet name]")
        sys.exit(1)
    
    # Create the url
    poet = sys.argv[1]
    savePoems(poet)
    
if __name__ == '__main__':
    main()
