import operator
import sys

def main():
  poet = sys.argv[1]
  distributionType = sys.argv[2]
  all_ = {}
  plural = {}
  plural["relative"] = "relatives"
  plural["richness"] = "richness"
  plural["weight"] = "weights"
  plural["gini"] = "ginis"
  plural["essential"] = "essentials"

  infile = open("poets/" + "local" + "/" + distributionType + "/" + poet + "_local_" + plural[distributionType] + ".txt", "r")
  
  for line in infile:
    args = line.strip().rstrip().split(" ")
    word = args[0]
    distr = float(args[1])
    all_[word] = distr
  sorted_all = sorted(all_.items(), key = operator.itemgetter(1), reverse=True)
  
  outfile = open("poets/" + "local" + "/" + distributionType + "/" + poet + "_local_" + plural[distributionType] + ".txt", "w")
  for elem in sorted_all:
    outfile.write(elem[0] + " " + str(elem[1]) + "\n")
    
if __name__ == '__main__':
    main()
