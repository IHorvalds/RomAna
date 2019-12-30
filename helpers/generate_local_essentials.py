# Role of this script: Use option 5 to generate all the frequencies of each poet.
import os
import sys

def main(distributionType, sortValues):
    num_lines = sum(1 for line in open('poets/poets.txt'))
    curr_count = 0
    file = open("poets/poets.txt", "r")
    for line in file:
        poet = line.strip().rstrip()
        if poet:
            print("Generating essentials for " + poet + "\n")
            command = "./gui 5 " + poet + " " + distributionType + " " + sortValues 
            os.system(command)
            curr_count += 1
            print("Total progess: [" + str(int(float(curr_count / num_lines) * 100)) + "%]")  
    
if __name__ == '__main__':
    # distributionType - richness or relative (preferably richness)
    # sortValues - sort the values by the ratios
    main(sys.argv[1], sys.argv[2])
