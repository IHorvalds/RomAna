from unidecode import unidecode
from urllib.parse import unquote

fread = open('../inflections.in', 'r')
fwrite = open('cleaned_inflections.in', 'w')

for line in fread:
    fwrite.write(unidecode(unquote(line)))
fread.close()
fwrite.close()

