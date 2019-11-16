library(ineq)
library(hash)

build_all_poets = function(distributionType) {
    # Init the hash table
    poetsToDistribution <- hash()

    count_poets = 0
    origFile = file(paste("poets/poets.txt"), "r")
    while (TRUE) {
        # Take the next poet
        poet = readLines(origFile, n = 1)
        if (length(poet) == 0)
            break;
        count_poets = count_poets + 1
        
        # Open the file of local distribution of the current poet
        inputFileName = paste("poets/local/", distributionType, "/", poet, "_local_", distributionType, "s.txt", sep="")
        inputFile = file(inputFileName, "r")
        
        # And read the file
        currHashTable <- hash()
        while (TRUE) {
            line = readLines(inputFile, n = 1);
            if (length(line) == 0)
                break;
            elems = strsplit(line, " ")[[1]]
            word = elems[1]
            distributionValue = as.double(elems[2])
            
            # Store the value into the hash table
            if (distributionType == "gini")
              currHashTable[[word]] <- 1 - distributionValue
            else
              currHashTable[[word]] <- distributionValue
        }
        close(inputFile)
        poetsToDistribution[[poet]] <- currHashTable
    }
    close(origFile)
    return(c(count_poets, poetsToDistribution))
}


build_global_words = function(poetsToDistribution) 
# Build the hash table with all words from out poets
{
    global_words <- hash()
    for (poet in keys(poetsToDistribution)) {
        hashTable = poetsToDistribution[[poet]]
        for (word in keys(hashTable))
            global_words[[word]] = TRUE
    }
    return(global_words)
}

build_poets_to_id = function(poetsToDistribution)
# Build a map name -> id for the poets
{
    poets_to_id <- hash()
    count = 0
    for (poet in keys(poetsToDistribution)) {
        poets_to_id[[poet]] = count
        count = count + 1
    }
    return(poets_to_id)
}

compute_global_gini = function(count_poets, idToDistribution)
# Given the values of the distribution of each poet, compute the gini of this distribution
{
    distr <- c()
    for (id in keys(idToDistribution)) {
        distr <- c(distr, idToDistribution[[id]])
    }
    completeBy = count_poets - length(distr)
    distr = c(rep(0, completeBy), distr)
    return(Gini(distr))
}

process = function(count_poets, distributionType, poetsToDistribution, global_words) 
# Given the hash table of distribution from all poets, compute the global gini for each word
{
    poetToId = build_poets_to_id(poetsToDistribution) 
    
    # Encode the name
    namesMap <- hash()
    namesMap[["relative"]] = "relative"
    namesMap[["gini"]] = "richness"
    
    # Open the file to store the global gini of the distribution in
    outputFileName <- paste("poets/global/gini_", namesMap[[distributionType]], ".txt", sep="")
    outputFile <- file(outputFileName, "w")
    
    # Compute for each the word the global gini
    for (word in keys(global_words)) {
        curr <- hash()
        for (poet in keys(poetsToDistribution))
            if (has.key(word, poetsToDistribution[[poet]]))
                curr[poetToId[[poet]]] = poetsToDistribution[[poet]][[word]]
        global_gini = compute_global_gini(count_poets, curr)
        write(paste(word, global_gini, sep=" "), outputFile, append=TRUE)
    }
    close(outputFile)
}

main <- function(argv)
{
    # argv[1] = which type of local distribution you want to use
    # By now, there are 2 available: 
    # "gini", "relative"
    
    options(warn = 1)
    distributionType <- argv[1]
    ret = build_all_poets(distributionType)
    process(ret[[1]], distributionType, ret[[2]], build_global_words(ret[[2]]))
    return(0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
