
void push_filtered_pairs(size_t length, token_t tokens[length], token_t left, token_t right, token_t replacement)
{
	size_t i = 0;
	while (i < length-1)
    	push_filtered_pairs_step(length, tokens, left, right, replacement, &i);
}
