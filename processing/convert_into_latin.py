import sys
from unidecode import unidecode
from urllib.parse import unquote

filename = sys.argv[1]
fread = open(filename, 'r')
lastOcc = filename.rfind('/')
latinPath = filename[:(lastOcc+1)] + "latin_" + filename[(lastOcc+1):]
fwrite = open(latinPath, 'w') 
for line in fread:
    fwrite.write(unidecode(unquote(line)))
fread.close()
fwrite.close()
