library(ineq)
library(hash)

build_local_distribution = function(poet, distributionType, shouldBeNormalized) {
  table <- hash()
  
  plural <- hash()
  plural[["gini"]] = "ginis"
  plural[["relative"]] = "relatives"
  plural[["richness"]] = "richness"
  
  inFileName = paste("poets/local/", distributionType, "/", poet, "_local_", plural[[distributionType]], ".txt", sep="")
  inFile = file(inFileName, "r");

  max = 0
  while (TRUE) {
    line = readLines(inFile, n = 1);
    if (length(line) == 0)
        break;
    elems = strsplit(line, " ")[[1]]
    word = elems[1]
    distributionValue = as.double(elems[2])
    if (distributionType == "gini")
        distributionValue <- 1 - distributionValue
    table[[word]] <- distributionValue
    if (distributionValue > max)
      max = distributionValue
  }
  # Normalize them to the maximum, if needed
  if (shouldBeNormalized)
    for (word in keys(table))
      table[[word]] = table[[word]] / max
  close(inFile)
  return(table)
}

build_global_distribution = function(distributionType, shouldBeNormalized) {
  table <- hash()
  
  inFileName = paste("poets/global/gini_", distributionType, ".txt", sep="")
  inFile = file(inFileName, "r");
  
  max = 0
  while (TRUE) {
      line = readLines(inFile, n = 1);
      if (length(line) == 0)
          break;
      elems = strsplit(line, " ")[[1]]
      word = elems[1]
      distributionValue = as.double(elems[2])
      table[[word]] <- distributionValue;
      if (distributionValue > max)
          max = distributionValue;
  }
  # Normalize them to maximum if necessary
  if (shouldBeNormalized)
      for (word in keys(table))
          table[[word]] = table[[word]] / max
  close(inFile)
  return(table)
}

golden_poet = function(poet) {
  # Build all the distributions
  local_richness <- build_local_distribution(poet, "gini", FALSE)
  local_relatives <- build_local_distribution(poet, "relative", FALSE)
  global_richness <- build_global_distribution("richness", FALSE)
  global_relatives <- build_global_distribution("relative", FALSE)
  
  # Open the file, into which the local weights will be written
  outFile <- file(paste("poets/local/weight/", poet, "_local_weights", ".txt", sep=""), "w")
  
  # Take into consideration only the words of the poet, not the global words
  for (word in keys(local_relatives)) {
    # Combine the global distributions into only one
    
    global_distribution <- global_richness[[word]]
    # global_distribution = sqrt(global_richness[[word]] * global_relatives[[word]])
    
    # And apply the function for computing the final weight
    x <- local_richness[[word]]
    y <- global_distribution
    z <- local_relatives[[word]]
    exp_x = as.double(1) # 1
    exp_y = as.double(0.02718) # euler number. Also 0.05
    exp_z = as.double(2)  # same degree as x, but upper. Also 10

    # The smaller the exponent, the more important the respective axis
    weight <- 1 / (exp(0.001 * ((1 / (1 - (1 - x) ^ exp_x)) + (1 / (1 - (1 - y) ^ exp_y)) + (1 / (1 - (1 - z) ^ exp_z)))))
    
    # weight <- 1 / (exp((1 / (1 - (1 - x) ^ 2)) + (1 / (1 - (1 - y) ^ 0.5))))
    
    # weight <- 1 / (exp(0.001 * ((1 / (1 - (1 - x) ^ 2)) + (1 / (1 - (1 - y) ^ 0.1)) + (1 / (1 - (1 - z) ^ 10)))))
    
    # And write the result
    write(paste(word, weight, sep=" "), outFile, append=TRUE)
  }
  close(outFile)
}

main <- function(argv)
{
    poet <- argv[1]
    golden_poet(poet)
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
