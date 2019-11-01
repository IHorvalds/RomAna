library(ineq)
library(hash)
library(tuple)

accumulate_all = function() {
    poetsToDistribution <- hash()
    # poetToCount <- hash()
    
    count_poets = 0
    allPoets = paste("poets/poets.txt")
    origFile = file(allPoets, "r")
    while (TRUE) {
        # Take the next poet
        poet = readLines(origFile, n = 1)
        if (length(poet) == 0)
            break;
        count_poets = count_poets + 1
        
        # Open the file of ginis for the current poet
        poetFile = paste("poets/gini/", poet, "_ginis.txt", sep="")
        con = file(poetFile, "r")
        
        # Read the configuration of the poet
        firstLine = readLines(con, n = 1);
        param = strsplit(firstLine, " ")[[1]]
        countPoems = as.numeric(param[1]);
        totalFreqs = as.numeric(param[2]);
        
        # poetToCount[[poet]] = countPoems
        
        currHashTable <- hash()
        
        # Read all the other lines
        while (TRUE) {
            line = readLines(con, n = 1);
            if (length(line) == 0)
                break;
            elems = strsplit(line, " ")[[1]]
            word = elems[1]
            gini = as.numeric(elems[2])
            
            # Fill the hash table with 1 - gini
            currHashTable[[word]] <- (1 - gini)
        }
        close(con)
        poetsToDistribution[[poet]] <- currHashTable
        
        # print(currHashTable[["el"]])
        # print(poetToGinis[[poet]][["el"]])
    }
    close(origFile)
    ret = c(count_poets, poetsToDistribution)
    return(ret)
}

get_poets_to_id = function(poetsToDistribution) {
    poets_to_id <- hash()
    count = 0
    for (poet in keys(poetsToDistribution)) {
        poets_to_id[[poet]] = count
        count = count + 1
    }
    return(poets_to_id)

}

get_global_words = function(poetsToDistribution) {
    global_words <- hash()
    for (poet in keys(poetsToDistribution)) {
        hashTable = poetsToDistribution[[poet]]
        for (word in keys(hashTable))
            global_words[[word]] = TRUE
    }
    return(global_words)
}

get_devs = function(count_poets, idToDistribution) {
    distr <- c()
    for (id in keys(idToDistribution)) {
        distr <- c(distr, idToDistribution[[id]])
    }
    completeBy = count_poets - length(distr)
    distr = c(rep(0, completeBy), distr)
    return(Gini(distr))
}

devs_all = function(count_poets, poetsToDistribution, global_words) {
    poetToId = get_poets_to_id(poetsToDistribution) 
    
    output <- file(paste("poets/words_distribution_gini.txt", sep=""), "w")
    
    for (word in keys(global_words)) {
        curr <- hash()
        for (poet in keys(poetsToDistribution)) {
            if (has.key(word, poetsToDistribution[[poet]])) {
                curr[poetToId[[poet]]] = poetsToDistribution[[poet]][[word]]
            }
        }
        global_gini = get_devs(count_poets, curr)
        write(paste(word, global_gini, sep=" "), output, append=TRUE)
    }
    close(output)
}

idf = function(count_poets, poetsToDistribution, global_words) {
    poetToId = get_poets_to_id(poetsToDistribution) 
    
    output <- file(paste("poets/words_distribution_idf.txt", sep=""), "w")
    
    for (word in keys(global_words)) {
        curr_count = 0
        for (poet in keys(poetsToDistribution)) {
            if (has.key(word, poetsToDistribution[[poet]])) {
                curr_count = curr_count + 1
            }
        }
        global_idf = log(as.numeric(count_poets) / curr_count)
        write(paste(word, global_idf, sep=" "), output, append=TRUE)
    }
    close(output)
}

main <- function(argv)
{
    options(warn=1)
    ret = accumulate_all()
    global_words = get_global_words(ret[[2]])
    devs_all(ret[[1]], ret[[2]], global_words)
    idf(ret[[1]], ret[[2]], global_words)
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
