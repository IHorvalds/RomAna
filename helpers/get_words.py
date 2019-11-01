import operator
import sys

def main():
    all_ = {}
    file = open("poets/words_distribution_" + sys.argv[1] + ".txt", "r")
    for line in file:
        args = line.strip().rstrip().split(" ")
        word = args[0]
        distr = float(args[1])
        all_[word] = distr
    sorted_all = sorted(all_.items(), key = operator.itemgetter(1))
    
    output = open("poets/sorted_distribution_" + sys.argv[1] + ".txt" , "w")
    for elem in sorted_all:
        output.write(str(elem) + "\n")
    
if __name__ == '__main__':
    main()
