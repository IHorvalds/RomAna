# The role of this script:
# Use the local frequencies of the poet and build the relative frequencies and the gini on these
# The relative frequencie is computed as the sum of ratios between the absolute frequencies and the size of the poem in which the word appears, everything above countPoems
library(ineq)

ginify_poet = function(poet) {
    inFile = file(paste("poets/local/frequency/", poet, "_local_frequencies.txt", sep=""), "r");
    firstLine = readLines(inFile, n = 1);
    param = strsplit(firstLine, " ")[[1]]
    
    # Open file to print the computed Gini coefficients
    giniOutput <- file(paste("poets/local/gini/", poet, "_local_ginis.txt", sep=""), "w")
    
    # Open file to print the computed relative frequencies
    relativeOutput <- file(paste("poets/local/relative/", poet, "_local_relatives.txt", sep=""), "w")
    
    # Read the configuration of the poet
    countPoems = as.numeric(param[1]);
    sizeOfPoems = as.numeric(tail(param, length(param) - 1))
    
    # Read all the other lines
    while (TRUE) {
        line = readLines(inFile, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        countOfPairs = as.numeric(elems[2])
        pairs = as.numeric(tail(elems, length(elems) - 2))
        
        relatives <- c()
        for (index in 0:countOfPairs - 1) {
            # Take the current poem
            poem <- pairs[2 * index + 2]
            
            # Compute the relative frequency (don't forget: R arrays start from 1)
            rel <- as.double(pairs[2 * index + 1] / sizeOfPoems[poem + 1])
            relatives <- c(relatives, rel)
        }
        # And compute the relative frequency of this word
        sum <- 1000 * sum(relatives) / countPoems
        
        # Fill with 0 for the poems in which the word is not present
        completeBy = countPoems - length(relatives)
        relatives <- c(rep(0, completeBy), relatives)
        
        # And print into the files
        write(paste(word, Gini(relatives), sep=" "), giniOutput, append=TRUE)
        write(paste(word, sum, sep=" "), relativeOutput, append=TRUE)
    }
    close(inFile)
    close(giniOutput)
    close(relativeOutput)
}

main <- function(argv)
{
    options(warn=1)
    ginify_poet(argv[1])
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
