## -*- coding: utf-8 -*-
from lxml import etree

class Word(object):
    """
    <word> <inflection count> <list of inflections>
    """
    def __init__(self, base, inflexList):
        self.base       = base
        self.inflexQ    = len(inflexList) ## inflection count
        self.inflexList = inflexList

    def __repr__(self):
        return self.base + " " + str(self.inflexQ) + " " + "".join(inflex + " " for inflex in self.inflexList)

    def __str__(self): ## I have no idea which one on these will get called when printing to the file. Fuck it.
        return self.base + " " + str(self.inflexQ) + " " + "".join(inflex + " " for inflex in self.inflexList)



class Parser(object):
    """
    Initialize with the path to the dexonline lexem xml file.
    """
    def __init__(self, path):
        self.pathToXML = path
        self.lexemList = [] ## List of representations of Word-s

        try:
            with open(path, "r") as xmlFile:
                xml         = xmlFile.read()
                utf8Parser  = etree.XMLParser(encoding="utf-8")
                print("lol")

                root        = etree.fromstring(xml.encode("utf-8"), parser=utf8Parser)
                print("lol2")


                lexemList   = []
                for lexem in root.getchildren():
                    form    = lexem.findtext("Form").replace("'", "") ## Base word

                    inflexList = []
                    for inflection in lexem.findall("InflectedForm"):
                        inflexList.append(inflection.findtext("Form").replace("'", ""))

                    lexemList.append(Word(form, inflexList))

                self.lexemList = lexemList
        except Exception as e:
            print(e)
            exit()

    def printToFile(self, printTo):
        with open(printTo, "w") as printer:
            printer.write(str(len(self.lexemList)) + "\n")
            printer.write("".join((str(word) + "\n") for word in self.lexemList))


def main():
    parser = Parser("2019-04-13-lexems.xml")
    parser.printToFile("inflections.in")

main()
