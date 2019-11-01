library(ineq)
library(hash)

get_devs = function(poet) {
    table <- hash()
    
    inFile = paste("poets/derivative/", poet, "_pure_devs.txt", sep="")
    con = file(inFile, "r");
    
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        distr = as.double(elems[2])
        table[[word]] <- distr;
    }
    close(con)
    return(table)
}

get_norm_devs = function(poet) {
    table <- hash()
    
    inFile = paste("poets/derivative/", poet, "_devs.txt", sep="")
    con = file(inFile, "r");
    
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        distr = as.double(elems[2])
        table[[word]] <- distr;
    }
    close(con)
    return(table)
}


get_distr = function(distribution_type) {
    table <- hash()
    
    inFile = paste("poets/words_distribution_", distribution_type, ".txt", sep="")
    con = file(inFile, "r");
    
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        distr = as.double(elems[2])
        table[[word]] <- distr;
    }
    close(con)
    return(table)
}

golden_poet = function(poet) {
    inFile = paste("poets/frequency/", poet, "_words_frequencies.txt", sep="")
    con = file(inFile, "r");
    firstLine = readLines(con, n = 1);
    param = strsplit(firstLine, " ")[[1]]
    
    output_normal <- file(paste("poets/normal/", poet, "_normal.txt", sep=""), "w")
    output_mixed_based <- file(paste("poets/final/", poet, "_mixed", ".txt", sep=""), "w")
    output_gini_based <- file(paste("poets/final/", poet, "_gini", ".txt", sep=""), "w") 
    output_idf_based <- file(paste("poets/final/", poet, "_idf", ".txt", sep=""), "w") 
    output_dev_based <- file(paste("poets/final/", poet, "_pure_dev", ".txt", sep=""), "w") 
    output_norm_dev_based <- file(paste("poets/final/", poet, "_norm_dev", ".txt", sep=""), "w") 
   
    # Read the configuration of the poet
    countPoems = as.numeric(param[1]);
    totalFreqs = as.numeric(param[2]);
    
    giniDistr = get_distr("gini")
    idfDistr = get_distr("idf")
    devDistr = get_devs(poet)
    normDevDistr = get_norm_devs(poet)
    
    # Read all the other lines
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        freqs = as.numeric(tail(elems, length(elems) - 2))
        currFreq = sum(freqs)
        
        gini_based = giniDistr[[word]] * currFreq
        idf_based = idfDistr[[word]] * currFreq
        dev_based = devDistr[[word]] * currFreq
        norm_dev_based = normDevDistr[[word]] * currFreq
        
        mixed_based = devDistr[[word]] * sqrt(idfDistr[[word]]) * currFreq
        
        write(paste(word, gini_based, sep=" "), output_gini_based, append=TRUE)
        write(paste(word, idf_based, sep=" "), output_idf_based, append=TRUE)
        write(paste(word, dev_based, sep=" "), output_dev_based, append=TRUE)
        write(paste(word, norm_dev_based, sep=" "), output_norm_dev_based, append=TRUE)
        
        write(paste(word, currFreq, sep=" "), output_normal, append=TRUE)
        write(paste(word, mixed_based, sep=" "), output_mixed_based, append=TRUE)
    }
    close(output_normal)
    close(output_gini_based)
    close(output_idf_based)
    close(output_dev_based)
    close(output_norm_dev_based)
    close(output_mixed_based)
}

main <- function(argv)
{
    # argv[1] = author
    # argv[2] = type of distribution
    # wordToDistr = get_distr(argv[2])
    golden_poet(argv[1])
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
