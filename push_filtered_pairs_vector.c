
void push_filtered_pairs(size_t length, token_t tokens[length], token_t left, token_t right, token_t replacement)
{
	size_t i = 0;
	while (i + VEC_SIZE < length-1)
	{
		vtok_t a = *(vtok_t*)&tokens[i];
		vtok_t b = *(vtok_t*)&tokens[i+1];
		vtok_t cond 
			= (a == right)
			| (a == replacement)
			| (b == left)
			| (b == replacement)
			;
		for (uint8_t j = 0; j < VEC_SIZE; j++)
		{
			if (cond[j])
			{
				size_t stop = i + VEC_SIZE;
				while (i < stop)
                	push_filtered_pairs_step(length, tokens, left, right, replacement, &i);
				goto next;
			}
		}
		i += VEC_SIZE;
	next:;
	}
	while (i < length-1)
    	push_filtered_pairs_step(length, tokens, left, right, replacement, &i);
}
