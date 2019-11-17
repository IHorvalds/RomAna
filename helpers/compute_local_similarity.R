library(ineq)
library(hash)

build_local_distribution = function(poet, distributionType, shouldBeNormalized) {
  table <- hash()
  
  plural <- hash()
  plural[["gini"]] = "ginis"
  plural[["relative"]] = "relatives"
  plural[["richness"]] = "richness"
  plural[["weight"]] = "weights"
  
  # For rank, use the ranking of the weights
  plural[["rank"]] = "weights"
  
  if (distributionType != "rank")
    inFileName = paste("poets/local/", distributionType, "/", poet, "_local_", plural[[distributionType]], ".txt", sep="")
  else
    inFileName = paste("poets/local/", "weight", "/", poet, "_local_", plural[[distributionType]], ".txt", sep="")
  inFile = file(inFileName, "r");

  max = 0
  rank <- 0
  while (TRUE) {
    line = readLines(inFile, n = 1);
    if (length(line) == 0)
        break;
    rank = rank + 1
    elems = strsplit(line, " ")[[1]]
    word = elems[1]
    distributionValue = as.double(elems[2])
    
    if (distributionType == "rank")
      distributionValue <- as.double(1 / rank)
    
    # Remark: if the distribution is gini, take the value as it is.
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

compute_total_weight = function(check, as, bs) {
  result <- as.double(0)
  if (check == FALSE) {
    common_keys <- intersect(keys(as), keys(bs))
    for (word in common_keys)
      result = result + as[[word]] * bs[[word]]
  } else {
    for (a in keys(as))
      result <- result + as[[a]] * as[[a]]
    result <- sqrt(result)
  }
  return(result)
}

apply_cosinus = function(poet, distributionType) {
  # Build all the distributions
  local_values <- build_local_distribution(poet, distributionType, FALSE)
  
  total_value <- compute_total_weight(TRUE, local_values, local_values)
  
  origFile = file(paste("poets/poets.txt"), "r")
  max <- 0
  best <- poet
  while (TRUE) {
    # Take the next poet
    curr_poet = readLines(origFile, n = 1)
    if (length(curr_poet) == 0)
        break;
    if (curr_poet != poet) {
      current_values <- build_local_distribution(curr_poet, distributionType, FALSE)
      curr_total_value <- compute_total_weight(TRUE, current_values, current_values)
      product <- compute_total_weight(FALSE, local_values, current_values)
      
      # This case appears when one poet does have only one poem
      if ((!curr_total_value) || (!total_value))
        similarity <- 0
      else
        similarity <- as.double(product / (curr_total_value * total_value))
      
      # Since cos(0), we take the maximum similarity
      if (similarity > max) {
        max = similarity
        best = curr_poet
      }
    }
  }
  print(paste(toupper(distributionType), ":", best, "->", max, sep = " "))
  close(origFile)
}

main <- function(argv)
{
    poet <- argv[1]
    distributionType <- argv[2]
    apply_cosinus(poet, distributionType)
    return (0);
}

if (identical (environment (), globalenv ()))
  quit (status=main (commandArgs (trailingOnly = TRUE)));
