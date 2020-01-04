# RomAna

We are analyzing the Romanian language and soon, not only.
The aim is to compute for each poet the most essential words.

This is your first time here?

## Clone the repository

```
1) git clone https://github.com/IHorvalds/RomAna.git
2) cd RomAna
```
## Update the Dexonline Data
Data could be obtained from "https://dexonline.ro/update5.php?last=0" (*big thanks to Cătălin Frâncu for his really great work*), where you should download the file of **lexems**. The first command downloads that file (please put in your **current date**).
```
1) wget -O dexdata/lexems.xml.gz https://dexonline.ro/static/download/xmldump/v5/[current-date: yyyy-mm-dd]-lexems.xml.gz
2) gunzip dexdata/lexems.xml.gz
3) python3 helpers/dexparser.py dexdata/lexems.xml dexdata/lexems.in
```
## Build up the InflexEngine
We store the lexems into a compressed trie, *InflexEngine*, the size of which is comparable to the size of the lexems file. In this way we ensure a fast preprocessing of the incoming texts.
```
1) make
2) ./gui 0 dexdata/lexems.in dictionary.bin
```
## Analyze the Poets
We have already some results (see *poets/local/essential*), but we are now improving the materials which we are working with, in order to obtain the best results.
