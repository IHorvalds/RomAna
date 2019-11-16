import operator
import sys

def main():
  all_ = {}
  max_ = 0
  file = open("poets/local/gini/" + sys.argv[1] + "_local_ginis.txt", "r")
  for line in file:
    args = line.strip().rstrip().split(" ")
    word = args[0]
    gini = float(args[1])
    all_[word] = 1 - gini
    if (1 - gini) > max_:
      max_ = 1 - gini
        
  sorted_all = sorted(all_.items(), key = operator.itemgetter(1), reverse=True)
  
  output = open("poets/local/richness/" + sys.argv[1] + "_local_richness.txt" , "w")
  for elem in sorted_all:
      output.write(elem[0] + " " + str(float(elem[1]) / max_) + "\n")
  
if __name__ == '__main__':
    main()
