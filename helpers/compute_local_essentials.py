import operator
import sys
from matplotlib import pyplot as plt
import numpy as np
from scipy.signal import argrelextrema
import statistics as stats
import math

def determineLastRank(fileName):
# Determine the last rank of word from this file
  file_ = open(fileName, "r")
  rank = 0
  lastDistr = -1
  for line in file_:
    args = line.strip().rstrip().split(" ")
    word = args[0]
    distr = float(args[1])
    if distr != lastDistr:
      lastDistr = distr
      rank += 1
  return rank

def computeEssentialWordsWithRank(poet, distributionType, procent, usePlot):
# Compute the most essential words of a poet, based on the distributionType, by analyzing only "procent" of his local list
  plural = {}
  plural["relative"] = "relatives"
  plural["richness"] = "richness"
  plural["weight"] = "weights"
  plural["gini"] = "ginis"
  plural["stop"] = "stops"
  
  globalFileName = "poets/" + "global" + "/" + "sorted_gini_" + distributionType + ".txt"
  localFileName = "poets/" + "local" + "/" + distributionType + "/" + poet + "_local_" + plural[distributionType] + ".txt"
  
  # First compute how many rankings the file contains (do not swap the order - this step must be done before actually storing the lines into the map)
  globalLastRank = determineLastRank(globalFileName)
  localLastRank = determineLastRank(localFileName)
  
  # If the file does not have so many ranks, take rankBound at its maximum, i.e, localLastRank
  if procent >= localLastRank:
    rankBound = localLastRank
  else:
    rankBound = int(round(procent / 100 * localLastRank))
  
  # Then read into the map
  globalFile = open(globalFileName, "r")
  
  all_ = {}
  realGlobalRank = 0
  lastGlobalDistr = -1
  for line in globalFile:
    args = line.strip().rstrip().split(" ")
    word = args[0]
    globalDistr = float(args[1])

    # Solves the problem: if 2 words have the same distribution, they should have the same rank as well
    if globalDistr != lastGlobalDistr:
      lastGlobalDistr = globalDistr
      realGlobalRank += 1
    all_[word] = realGlobalRank

  # rankGlobalFile ... globalLastRank
  # x              ... localLastRank
  # x = localLastRank * rankGlobalFile / globalLastRank
  
  # Remark: realRank is not the same with lineCounter
  lineCounter = 1
  realLocalRank = 0
  xs = []
  ys = []
  lastLocalDistr = -1
  keepLinkWithWord = {}
  
  localFile = open(localFileName, "r")
  for line in localFile:
    if realLocalRank > rankBound:
      break
    args = line.strip().rstrip().split(" ")
    word = args[0]
    localDistr = float(args[1])
    
    # Solves the problem: if 2 words have the same distribution, they should have the same ranking as well
    if localDistr != lastLocalDistr:
      lastLocalDistr = localDistr
      realLocalRank += 1
    
    # Compute the ratio between the y-axis and x-axis
    # See the formula above in order to resize the shape of the y-axis
    
    # We may use sqrt in order to balance the powers of the values < 1 and > 1
    val = math.sqrt(float((localLastRank * all_[word] / globalLastRank) / realLocalRank))
    
    # Why not add realLocalRank? The problem is that we should have a bijection word <-> ranking,
    # so if we use realRank that would not be the case
    xs.append(lineCounter)
    ys.append(val)
    
    # Keep the link lineCounter -> word through a map
    keepLinkWithWord[lineCounter] = word
    lineCounter += 1
  
  # Compute the local maximum extremas
  # The function to be analyzed is localRank -> globalRank / localRank
  tmpX = np.array(xs)
  tmpY = np.array(ys)
  max_extremas = argrelextrema(tmpY, np.greater)
  peaksX = tmpX[np.array(max_extremas)][0]
  peaksY = tmpY[np.array(max_extremas)][0]
  
  # If no extrema has been found, then choose the maximum at the edges
  if len(peaksY) == 0:
    resolveX = []
    resolveY = []
    if len(tmpY) == 1:
      resolveY.append(tmpY[0])
      resolveX.append(tmpX[0])
    else:
      # Check the front of tmpY
      if tmpY[0] >= tmpY[1]:
        resolveY.append(tmpY[0])
        resolveX.append(tmpX[0])
      # Check the bottom of tmpY
      len_ = len(tmpY)
      if tmpY[len_ - 1] >= tmpY[len_ - 2]:
        resolveY.append(tmpY[len_ - 1])
        resolveX.append(tmpX[len_ - 1])
    peaksY = np.array(resolveY)
    peaksX = np.array(resolveX)
  
  # Take the mean of the peaks
  meanPeaks = stats.mean(peaksY)
  
  # And print the most essential words along with their y-values
  outputFileName = "poets/local/essential/" + poet + "_local_essentials.txt"
  epsilon = 0.001
  print("Filter all above: " + str(meanPeaks))
  output = open(outputFileName, "w")
  for index in range(0, len(tmpY)):
    if tmpY[index] - meanPeaks >= -epsilon:
      output.write(keepLinkWithWord[tmpX[index]] + " " + str(tmpY[index]) + "\n")
  output.close()
  
  # For plotting
  if usePlot:
    # Draw a line where the mean of the peaks lies
    lineX = np.linspace(0, lineCounter, lineCounter)
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
  return

if __name__ == '__main__':
  # python3 eminescu richness 20 0 - will compute the most essential words for Eminescu, based on the Gini's coefficient, by analyzing only the 20% of the list of his local ginis.
  # python3 emienscu relative 20 0 - same but for relative frequencies. So for the local analysis only the frequency is relevant.
  if len(sys.argv) == 4:
    computeEssentialWordsWithRank(sys.argv[1], sys.argv[2], float(sys.argv[3]), False)
  elif len(sys.argv) == 5:
    computeEssentialWordsWithRank(sys.argv[1], sys.argv[2], float(sys.argv[3]), int(sys.argv[4]) != 0)
  else:
    print("python3 helpers/compute_local_essentials.py poet distributionType procent usePlot")
