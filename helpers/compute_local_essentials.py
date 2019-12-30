import operator
import sys
from matplotlib import pyplot as plt
import numpy as np
from scipy.signal import argrelextrema
import statistics as stats
import math

def determineLength(fileName):
# Determine the count of lines of a file
    file_ = open(fileName, "r")
    count = 0
    for line in file_:
        count += 1
    return count

def computeEssentialWords(poet, distributionType, procent, usePlot):
# Compute the most essential words of a poet, based on the distributionType, by analyzing only procent of his local list
  plural = {}
  plural["relative"] = "relatives"
  plural["richness"] = "richness"
  plural["weight"] = "weights"
  plural["gini"] = "ginis"
  plural["stop"] = "stops"
  
  globalFileName = "poets/" + "global" + "/" + "sorted_gini_" + distributionType + ".txt"
  localFileName = "poets/" + "local" + "/" + distributionType + "/" + poet + "_local_" + plural[distributionType] + ".txt"
  
  # First compute how many lines the file contains (do not swap the order - this step must be done before actually storing the lines into the map)
  globalSize = determineLength(globalFileName)
  localSize = determineLength(localFileName)
  rankBound = int(round(procent / 100 * localSize))
  
  # Then read into the map
  globalFile = open(globalFileName, "r")
  localFile = open(localFileName, "r")
  
  all_ = {}
  rank = 1
  for line in globalFile:
    args = line.strip().rstrip().split(" ")
    word = args[0]
    distr = float(args[1])
    all_[word] = rank
    rank += 1


  # positionGlobalFile ... globalSize
  # x                  ... localSize
  # x = localSize * positionGlobalFile / globalSize
  
  rank = 1
  xs = []
  ys = []
  keepLinkWithWord = {}
  for line in localFile:
    if rank == rankBound:
      break
    args = line.strip().rstrip().split(" ")
    word = args[0]
    
    # Compute the ratio between the y-axis and x-axis
    # See the formula above in order to resize the shape of the y-axis
    
    # We may use sqrt in order to balance the powers of the values < 1 and > 1
    val = math.sqrt(float((localSize * all_[word] / globalSize) / rank))
    xs.append(rank)
    ys.append(val)
    
    # Keep the link rank -> word through a map
    keepLinkWithWord[rank] = word
    rank += 1
  
  # Compute the local maximum extremas
  # The function to be analyzed is localRank -> globalRank / localRank
  max_extremas = argrelextrema(np.array(ys), np.greater)
  peaksX = np.array(xs)[np.array(max_extremas)][0]
  peaksY = np.array(ys)[np.array(max_extremas)][0]
  meanPeaks = stats.mean(peaksY)
  
  # And print the most essential words along with their y-values
  outputFileName = "poets/local/essential/" + poet + "_local_essentials.txt"
  output = open(outputFileName, "w")
  tmpX = np.array(xs)
  tmpY = np.array(ys)
  epsilon = 0.001
  print("Filter all above: " + str(meanPeaks))
  for index in range(0, len(tmpY)):
    if tmpY[index] - meanPeaks >= -epsilon:
      output.write(keepLinkWithWord[tmpX[index]] + " " + str(tmpY[index]) + "\n")
  output.close()
  
  # For plotting
  if usePlot:
    # Draw a line where the mean of the peaks lies
    lineX = np.linspace(0, rank, rank)
    lineY = 0 * lineX + meanPeaks
    
    # Print all the points
    plt.plot(xs, ys)
    
    # Also plots the chosen peaks (deactivated)
    plt.plot(peaksX, peaksY, 'o', color='black')
    
    # Print the red line of the mean
    plt.plot(lineX, lineY, '-r')
    
    plt.xlabel("Rank of word in the list of local ginis")
    plt.ylabel("Ratio between globalRank and localRank")
    plt.show()
    
if __name__ == '__main__':
  # python3 eminescu richness 20 0 - will compute the most essential words for Eminescu, based on the Gini's coefficient, by analyzing only the 20% of the list of his local ginis.
  # python3 emienscu relative 20 0 - same but for relative frequencies. So for the local analysis only the frequency is relevant.
  if len(sys.argv) == 4:
    computeEssentialWords(sys.argv[1], sys.argv[2], float(sys.argv[3]), False)
  elif len(sys.argv) == 5:
    computeEssentialWords(sys.argv[1], sys.argv[2], float(sys.argv[3]), True)
  else:
    print("python3 helpers/compute_local_essentials.py poet distributionType procent usePlot")
