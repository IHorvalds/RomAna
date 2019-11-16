# Role of this script: Use option 3 to generate all the frequencies of each poet.
import os

def main():
    num_lines = sum(1 for line in open('poets/poets.txt'))
    curr_count = 0
    file = open("poets/poets.txt", "r")
    for line in file:
        poet = line.strip().rstrip()
        if poet:
            print("Generating frequencies for " + poet + "\n")
            command = "./gui 3 " + poet
            os.system(command)
            curr_count += 1
            print("Total progess: [" + str(int(float(curr_count / num_lines) * 100)) + "%]")  
    
if __name__ == '__main__':
    main()
