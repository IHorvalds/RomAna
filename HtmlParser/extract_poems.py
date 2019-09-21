# The role of this script:
# Uses the site romanianvoice.com to take all the poem titles of the poet from the input
# TODO: for a specific author, put in a folder with his name separated files with his poems
import urllib.request
import html
import sys

def applyDiacritics(text):
    # Transform the slash errors from utf-8 decode into diacritics
    
    # Small diacritics
    text = text.replace("\\xe2", "â")
    text = text.replace("\\xee", "î")
    text = text.replace("\\xe3", "ă")
    text = text.replace("\\xfe", "ţ")
    text = text.replace("\\xba", "ş")

    # Big diacritics
    text = text.replace("\\xaa", "Ş")
    text = text.replace("\\xce", "Î")
    text = text.replace("\\xde", "Ţ")
    return text

def keepPoemRefs(text):
    keepedLines = []
    lines = text.splitlines()
    for line in lines:
        if line.find("<a href=\"../poezii/") != -1:
            keepedLines.append(line)
    return keepedLines

def getPoemNames(refLines):
    poemNames = []
    for ref in refLines:
        pos = ref.find(".php")
        if pos == -1:
            print("We got a problem at parsing the poem reference")
            sys.exit(1)
        else:
            
            ref = ref[:pos]
            lastOcc = ref.rfind("/")
            ref = ref[(lastOcc + 1):]
            poemNames.append(ref)
    return poemNames

def writePoemNames(poemNames, fileName):
    f = open(fileName, "w")
    for name in poemNames:
        f.write(name + '\n')
    pass

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 " + sys.argv[0] + " [poet name]")
        sys.exit(1)
    
    # Create the url
    poet = sys.argv[1]
    url = "http://www.romanianvoice.com/poezii/poeti/" + poet + ".php"

    # Connect to the site
    request = urllib.request.Request(url)
    response = urllib.request.urlopen(request)
    
    # Decode the site into utf-8
    text = response.read().decode('utf-8', errors = 'backslashreplace')

    # Write the titles of the poems
    writePoemNames(getPoemNames(keepPoemRefs(text)), "list_poems.txt")
    
if __name__ == '__main__':
    main()
