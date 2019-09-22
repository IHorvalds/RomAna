# Role of this script: simply computes the avg length of all poems
# Results on 22.09.2019 - 1500 poems, 227907 words -> avg = 151,938 words/poem
import extract_poetry

f = open("../poets.txt", "r")

sum = 0
count = 0
for line in f:
    poet = line.strip()
    filename = "../poets/" + poet + "_list_poems.txt"
    tmp = open(filename, "r")
    for line2 in tmp:
        poem = line2.strip()
        currPoetry = extract_poetry.getPoetry(poem)
        count += 1
        sum += len(currPoetry.split())
        
print("count = " + str(count) + " and sum = " + str(sum))
        
