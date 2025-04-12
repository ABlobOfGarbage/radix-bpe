# radix-bpe
Byte pair encoding implemented with radix sort

## The procedure

To find the most frequent pairs do:
 - Iterate through the tokens and append every single pair to an array
 - Sort that array using radix sort
 - Iterate through the sorted array and count how many times the same pair occurs in a row, this is that pair's frequency
 - Pick the most frequent pair and replace all occurances of it in the text with a new token

It doesn't just keep track of the single most frequent pair, instead it keeps track of several of the next most frequent pairs as well (in a queue). When the replacement is done it might affect some of the counts in that queue, so after it picks the most frequent pair it filters out any pairs from the queue that could be affected. (If we have the token *lr*, it filters out any pairs that end in *l* or begin with *r*.) After the replacement is done it doesn't push all pairs to the array again, only those involving the new token or that were filtered out since their counts could have changed. Again the pairs are sorted and counted and added to the (now partially full) queue.

It doesn't take command line arguments, if you want to change where it reads/writes input/results to you have to change the code yourself.

I preemptively apologize for the mess that is this code, ask me to clarify anything and I will try my best to. Also I don't really use git so I'm sure I've done something wrong here too but whatever.

## The speed

In my testing I use a ~10 megabyte text file that I scraped from some twitch streams. This program is able to process that file in less than 3 minutes. I would say that is pretty fast. I have not implemented multithreading yet but I have done vectorization.
