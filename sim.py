import sys
import difflib
import re
from difflib import SequenceMatcher

def lcs(a, b):
  # generate matrix of length of longest common subsequence for substrings of both words
  lengths = [[0] * (len(b)+1) for _ in range(len(a)+1)]
  for i, x in enumerate(a):
    for j, y in enumerate(b):
      if x == y:
        lengths[i+1][j+1] = lengths[i][j] + 1
      else:
        lengths[i+1][j+1] = max(lengths[i+1][j], lengths[i][j+1])

  # read a substring from the matrix
  result = ''
  j = len(b)
  for i in range(1, len(a)+1):
    if lengths[i][j] != lengths[i-1][j]:
      result += a[i-1]
  return len(result)

def similar(a, b):
  return SequenceMatcher(None, a, b).ratio()

def lcs(a, b):
  # generate matrix of length of longest common subsequence for substrings of both words
  lengths = [[0] * (len(b)+1) for _ in range(len(a)+1)]
  for i, x in enumerate(a):
    for j, y in enumerate(b):
      if x == y:
        lengths[i+1][j+1] = lengths[i][j] + 1
      else:
        lengths[i+1][j+1] = max(lengths[i+1][j], lengths[i][j+1])

  # read a substring from the matrix
  result = ''
  j = len(b)
  for i in range(1, len(a)+1):
    if lengths[i][j] != lengths[i-1][j]:
      result += a[i-1]
  return len(result)

def sets():
  with open("poets/list_poets.txt", "r") as file:
    poets = []
    for line in file:
      poets.append(line.strip())
    matches = {}
    for poet in poets:
      tmp = poets
      tmp.remove(poet)
      list = difflib.get_close_matches(poet, tmp)
      if list != []:
        matches[poet] = list[0]
    elems = matches.items()
    elems.sort(key = lambda x: float(lcs(x[0], x[1]) / max(len(x[0]), len(x[1]))))
    for key, value in matches.items():
      print(key + " " + value)
    print(elems)
  pass

def jaccard_similarity(list1, list2):
    intersection = len(list(set(list1).intersection(list2)))
    union = (len(list1) + len(list2)) - intersection
    return float(intersection) / union

def check(a, b):
  return float(lcs(a, b) / min(len(a), len(b)))

def once(a, b):
  if similar(a, b) == 1:
    return similar(a, b)
  else:
    if jaccard_similarity(a, b) == 1:
      return jaccard_similarity(a, b)
    else:
      return similar(a, b)
  
def process(xs, ys):
  # len(xs) < len(ys)
  total = 0
  offset = 0
  flag = False
  count = 0
  for i in range(0, len(xs)):
    if i + offset == len(ys):
      break
    count += 1
    curr = once(xs[i], ys[i + offset])
    next = 0
    val = 0
    if i + offset + 1 < len(ys):
      next = once(xs[i], ys[i + offset + 1])
    if next > curr:
      offset += 1
      val = next
    else:
      val = curr
    total += val
    if val == 0:
      flag = True
  if flag == True:
    return 0
  return total / count

def put(text):
  # Small diacritics
  text = text.replace("%E2", "â")
  text = text.replace("%EE", "î")
  text = text.replace("%E3", "ă")
  text = text.replace("%FE", "ţ")
  text = text.replace("%BA", "ş")

  # Big diacritics
  text = text.replace("%C3", "Ă")
  text = text.replace("%C2", "Â")
  text = text.replace("%AA", "Ş")
  text = text.replace("%CE", "Î")
  text = text.replace("%DE", "Ţ")
  
  # Others
  text = text.replace("%2C", ",")
  text = text.replace("%96", "-")
  return text

def get(a):
  a = put(a)
  return re.findall(r"[\w']+", a)

def precise(a, b):
  xs = get(a)
  ys = get(b)
  
  # print(a + " -> " + str(xs))
  # print(b + " -> " + str(ys))
  
  total = 0
  if len(xs) < len(ys):
    total = process(xs, ys)
  elif len(xs) > len(ys):
    total = process(ys, xs)
  else:
    total = (process(xs, ys) + process(ys, xs)) / 2
  return total

def combine(a, b):
  return precise(a, b)

def main():
  with open("saved.txt", "r") as file:
    elems = []
    for line in file:
      args = line.strip().rstrip().split(" ")
      poet1 = args[0]
      poet2 = args[1]
      
      elems.append((poet1, poet2))
    elems = sorted(elems, key = lambda x: combine(x[0], x[1]))
    for elem in elems:
      print(put(elem[0]) + " " + put(elem[1]) + " with: " + str(combine(elem[0], elem[1])))
    
if __name__ == '__main__':
  main()

