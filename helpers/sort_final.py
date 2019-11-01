import operator
import sys

def main():
    # sys.argv[1] = poet
    # sys.argv[2] = which folder
    # sys.argv[3] = which distribution
    all_ = {}
    file = open("poets/" + sys.argv[2] + "/" + sys.argv[1] + "_" + sys.argv[3] + ".txt", "r")
    for line in file:
        args = line.strip().rstrip().split(" ")
        word = args[0]
        distr = float(args[1])
        all_[word] = distr
    sorted_all = sorted(all_.items(), key = operator.itemgetter(1), reverse=True)
    
    output = open("poets/" + sys.argv[2] + "/" + sys.argv[1] + "_" + sys.argv[3] + ".txt" , "w")
    for elem in sorted_all:
        output.write(elem[0] + " " + str(elem[1]) + "\n")
    
if __name__ == '__main__':
    main()
