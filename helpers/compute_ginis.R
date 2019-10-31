library(ineq)

ginify_poet = function(poet) {
    inFile = paste("poets/frequency/", poet, "_words_frequencies.txt", sep="")
    con = file(inFile, "r");
    firstLine = readLines(con, n = 1);
    param = strsplit(firstLine, " ")[[1]]
    
    
    output <- file(paste("poets/gini/", poet, "_ginis.txt", sep=""), "w")
    write(firstLine, output)
    
    # Read the configuration of the poet
    countPoems = as.numeric(param[1]);
    totalFreqs = as.numeric(param[2]);
    
    # Read all the other lines
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        freqs = as.numeric(tail(elems, length(elems) - 2))
        completeBy = countPoems - length(freqs)
        freqs = c(rep(0, completeBy), freqs)
        write(paste(word, Gini(freqs), sep=" "), output, append=TRUE)
    }
}

main <- function(argv)
{
    ginify_poet(argv[1])
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
