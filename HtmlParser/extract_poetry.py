# The role of this script:
# Uses the site romanianvoice.com to take only the poetry out of the html file
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

def keepOnlyPoetry(text):
    # The utf-8 decoded file has some problems. First solve them
    text = applyDiacritics(text)
    
    ## Observation: the poetry is delimitated by the first <br> 
    # and the next "<font" or the next "<hr"
    
    # Find the first <br>. From that point begins the poetry
    brClause = "<br>"
    text = text[(text.find(brClause) + len(brClause)):]
    
    # Find the next <fond>. At that point ends the poetry
    fontClause = "<font"
    fontPos = text.find(fontClause)
    
    # Find the next <hr>. At that point ends the poetry
    hrClause = "<hr";
    hrPos = text.find(hrClause)
    
    lastPos = -1
    if fontPos == -1:
        lastPos = hrPos
    elif hrPos == -1:
        lastPos = fondPos
    else:
        lastPos = min(fontPos, hrPos)
    
    # Cut if any variant was found
    if lastPos != -1:
        text = text[:lastPos]
    
    # Clean any brClauses
    text = text.replace(brClause, "")
    
    # Continue with a thread-based algorithm
    # How many opened "<" are currenly in the text
    countWriters = 0
    poetry = ""
    for c in text:
        # Start a writer
        if c == '<':
            countWriters += 1
        elif c == '>':
            # Check if have many more unclosed writers
            if countWriters != 0:
                countWriters -= 1
        # No writers? Then we have a free text, which should be the poetry
        elif countWriters == 0:
            poetry += c
    return poetry
    
def main():
    if len(sys.argv) < 2:
        print("Usage: python3 " + sys.argv[0] + " [poetry name]")
        sys.exit(1)
    
    # Create the url
    poetry = sys.argv[1]
    url = "http://www.romanianvoice.com/poezii/poezii/" + poetry + ".php"

    # Connect to the site
    request = urllib.request.Request(url)
    response = urllib.request.urlopen(request)
    
    # Decode the site into utf-8
    # When utf-8 sends an error, put instead a slash encoding
    text = response.read().decode('utf-8', errors = 'backslashreplace')
    
    # Print the poetry in "poetry".out
    poetryFile = open(poetry + ".txt", "w")
    poetryFile.write(keepOnlyPoetry(text))

if __name__ == '__main__':
    main()
