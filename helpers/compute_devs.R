library(ineq)
library(hash)
library(tuple)
library(polynom)
library(PolynomF)

compute_rel_freqs = function(poet) {
    inFile = paste("poets/frequency/", poet, "_words_frequencies.txt", sep="")
    con = file(inFile, "r");
    firstLine = readLines(con, n = 1);
    param = strsplit(firstLine, " ")[[1]]
   
    # Read the configuration of the poet
    countPoems = as.numeric(param[1]);
    totalFreqs = as.numeric(param[2]);
    
    # Read all the other lines
    ret <- hash()
    while (TRUE) {
        line = readLines(con, n = 1);
        if (length(line) == 0)
            break;
        elems = strsplit(line, " ")[[1]]
        word = elems[1]
        freqs = as.numeric(tail(elems, length(elems) - 2))
        currFreq = sum(freqs)
        ret[[word]] <- (currFreq / totalFreqs)
    }
    close(con)
    return(ret)
}

accumulate_all = function() {
    poetsToDistribution <- hash()
    poetsToRelative <- hash()
    
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
        poetsToRelative[[poet]] <- compute_rel_freqs(poet)
        
        # print(currHashTable[["el"]])
        # print(poetToGinis[[poet]][["el"]])
    }
    close(origFile)
    ret = c(count_poets, poetsToDistribution, poetsToRelative)
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

revert_table = function(poetToId) {
    id_to_poet <- hash()
    for (poet in keys(poetToId)) {
        id_to_poet[[toString(poetToId[[poet]])]] = poet
    }
    return(id_to_poet)
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

get_global_gini = function(count_poets, idToDistribution) {
    distr <- c()
    for (id in keys(idToDistribution)) {
        distr <- c(distr, idToDistribution[[id]])
    }
    completeBy = count_poets - length(distr)
    distr = c(rep(0, completeBy), distr)
    return(Gini(distr))
}

gini = function(count_poets, poetsToDistribution, global_words) {
    poetToId = get_poets_to_id(poetsToDistribution) 
    
    output <- file(paste("poets/words_distribution_gini.txt", sep=""), "w")
    
    for (word in keys(global_words)) {
        curr <- hash()
        for (poet in keys(poetsToDistribution)) {
            if (has.key(word, poetsToDistribution[[poet]])) {
                curr[poetToId[[poet]]] = poetsToDistribution[[poet]][[word]]
            }
        }
        global_gini = get_global_gini(count_poets, curr)
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

get_devs = function(word, count_poets, idToDistr) {
    # Transform the hash table into a matrix
    keys_ = ls(idToDistr)
    vals_ = c()
    for (id in keys_)
        vals_ = c(vals_, idToDistr[[id]])
    if (length(keys_) == 1) {
        # count_poets is the derivate of the function f(x) = ax + b
        # with a = count_poets and b = 1 - count_poets
        ret <- hash()
        ret[[keys_[1]]] <- count_poets
        return(ret)
    } else {
        mat = matrix(c(keys_, vals_, rep(0, length(keys_))), ncol = 3)
        # print(m)
        
        # Sort the distribution value increasingly
        mat = mat[order(mat[,2]),]
    
        totalSum = as.double(0)
        for (row in 1:nrow(mat))
            totalSum = totalSum + as.double(mat[row, 2])
        
        sum = as.double(0)
        for(row in 1:nrow(mat)) {
            sum = sum + as.double(mat[row, 2])
            mat[row, 2] = as.double(sum / totalSum)
        }
        
        # Create the axis
        eps = 1 / count_poets
        mul = count_poets - nrow(mat)
        
        xs <- c(mul * eps)
        ys <- c(0)
        
        # Go one step further, because 0 has already been addedF
        mul = mul + 1
        for(row in 1:nrow(mat)) {
            mat[row, 3] = as.double(mul * eps)
            mul = mul + 1
            xs <- c(xs, as.double(mat[row, 3]))
            ys <- c(ys, as.double(mat[row, 2]))
        }
        
        # Tune the degree
        degree <- min(length(xs) - 1, 4)
        fitted <- poly_from_values(xs, ys)
        # fitted <- as_polynom(as.vector(coef(lm(ys ~ poly(xs, degree, raw=TRUE)))))
        
        dev <- deriv(fitted)

        if (word == "implora") {
            print(fitted)
            plot(mat[,3], as.function(fitted)(as.double(mat[, 3])))
            plot(mat[,3], as.function(dev)(as.double(mat[, 3])))
            
            for(row in 1:nrow(mat)) {
                now <- as.function(fitted)(as.double(mat[row, 3]))
                print(paste(now, "vs", mat[row, 2], sep = " "))
            }
        }
        
        ret <- hash()
        for(row in 1:nrow(mat)) {
            ret[[mat[row, 1]]] <- as.function(dev)(as.double(mat[row, 3]))
        }
        return(ret)
    }
}

devs_all = function(count_poets, poetsToDistribution, poetsToRelative, global_words) {
    poetToId = get_poets_to_id(poetsToDistribution) 
    idToPoet = revert_table(poetToId)
    
    for (word in keys(global_words)) {
        curr <- hash()
        for (poet in keys(poetsToDistribution)) {
            if (has.key(word, poetsToDistribution[[poet]])) {
                curr[as.numeric(poetToId[[poet]])] = as.numeric(poetsToDistribution[[poet]][[word]]) * as.numeric(poetsToRelative[[poet]][[word]])
            }
        }
        
        if (word == "implora")
            print(curr)
        
        # Compute the derivatives on Lorenz curve
        devs = get_devs(word, count_poets, curr)
    
        if (word == "implora")
            print(devs)
    
        if (TRUE) {
            # Normalize the derivatives
            sum = as.double(0)
            for (dev in keys(devs))
                sum = sum + as.double(devs[[dev]])
            for (dev in keys(devs))
                devs[[dev]] = devs[[dev]] / sum
        }
        
        # Update the values for each poet with this word
        for (dev in keys(devs)) {
            poetsToDistribution[[ idToPoet[[dev]] ]][[word]] <- devs[[dev]]
        }
    }
    
    print("All derivatives computed. Now copy into files")
    
    for (poet in keys(poetsToDistribution)) {
        output <- file(paste("poets/derivative/", poet, "_rel_simple_devs.txt", sep=""), "w")
        for (word in keys(poetsToDistribution[[poet]])) {
            write(paste(word, poetsToDistribution[[poet]][[word]], sep=" "), output, append=TRUE)
        }
        print(paste("Derivatives of", poet, "done", sep = " "))
        close(output)
    }
}

main <- function(argv)
{
    options(warn=1)
    ret = accumulate_all()
    global_words = get_global_words(ret[[2]])
    # gini(ret[[1]], ret[[2]], global_words)
    # idf(ret[[1]], ret[[2]], global_words)
    devs_all(ret[[1]], ret[[2]], ret[[3]], global_words)
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
