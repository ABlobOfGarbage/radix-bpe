
#define REPLACE_PAIR_LOOK_AHEAD 8
#define REPLACE_PAIR_STEP_SIZE (VEC_SIZE*REPLACE_PAIR_LOOK_AHEAD)

size_t replace_pair(size_t length, token_t tokens[length], token_t l, token_t r, token_t replacement)
{
	token_t *dst = tokens;
	token_t *src = tokens;
	token_t *end = tokens + length - 1;

	while (src + REPLACE_PAIR_STEP_SIZE < end)
	{
		vtok_t cond = {};
		for (size_t i = 0; i < REPLACE_PAIR_LOOK_AHEAD; i++)
		{
    		vtok_t a = *(vtok_t*)(i*VEC_SIZE + src);
    		vtok_t b = *(vtok_t*)(i*VEC_SIZE + src+1);
    		cond |= (a == l) & (b == r);
		}
		for (uint8_t i = 0; i < VEC_SIZE; i++)
		{
			if (cond[i])
			{
				token_t *stop = src + REPLACE_PAIR_STEP_SIZE;
				while (src < stop)
                    replace_pair_step(&dst, &src, l, r, replacement);
				goto next;

			}
		}
		for (size_t i = 0; i < REPLACE_PAIR_LOOK_AHEAD; i++)
    		*(vtok_t*)(dst + i*VEC_SIZE) = *(vtok_t*)(src + i*VEC_SIZE);
		dst += REPLACE_PAIR_STEP_SIZE;
		src += REPLACE_PAIR_STEP_SIZE;
	next:;
	}

	while (src < end)
        replace_pair_step(&dst, &src, l, r, replacement);

	if (src == end)
    	*(dst++) = *src;

	return ((size_t)dst - (size_t)tokens) / sizeof(token_t);
}

