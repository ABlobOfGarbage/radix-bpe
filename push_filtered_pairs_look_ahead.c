
#define PUSH_PAIRS_LOOK_AHEAD 4

void push_filtered_pairs(size_t length, token_t tokens[length], token_t left, token_t right, token_t replacement)
{
	size_t i = 0;
	while (i + PUSH_PAIRS_LOOK_AHEAD < length-1)
	{
		for (size_t j = 0; j < PUSH_PAIRS_LOOK_AHEAD; j++)
    		if (tokens[i+j] == right)
        		goto do_step;
		for (size_t j = 0; j < PUSH_PAIRS_LOOK_AHEAD; j++)
    		if (tokens[i+j+1] == left)
        		goto do_step;
		for (size_t j = 0; j < PUSH_PAIRS_LOOK_AHEAD+1; j++)
    		if (tokens[i+j] == replacement)
        		goto do_step;
		i += PUSH_PAIRS_LOOK_AHEAD;
		continue;
	do_step:
    	size_t end = i + PUSH_PAIRS_LOOK_AHEAD;
		while (i < end)
    		push_filtered_pairs_step(length, tokens, left, right, replacement, &i);
	}
	while (i < length-1)
    	push_filtered_pairs_step(length, tokens, left, right, replacement, &i);
}
