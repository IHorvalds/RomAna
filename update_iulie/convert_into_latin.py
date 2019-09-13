import sys
from unidecode import unidecode
from urllib.parse import unquote


filename = sys.argv[1]
fread = open(filename, 'r')
fwrite = open('latin_' + filename, 'w') 
for line in fread:
    fwrite.write(unidecode(unquote(line)))
fread.close()
fwrite.close()
