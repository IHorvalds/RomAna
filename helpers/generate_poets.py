import os

def main():
    num_lines = sum(1 for line in open('poets.txt'))
    curr_count = 0
    file = open("poets.txt", "r")
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
