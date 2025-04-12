
size_t replace_pair(size_t length, token_t tokens[length], token_t l, token_t r, token_t replacement)
{
	token_t *dst = tokens;
	token_t *src = tokens;
	token_t *end = tokens + length - 1;

	while (src + VEC_SIZE < end)
	{
		vtok_t a = *(vtok_t*)src;
		vtok_t b = *(vtok_t*)(src+1);
		vtok_t cond = (a == l) & (b == r);
		for (uint8_t i = 0; i < VEC_SIZE; i++)
		{
			if (cond[i])
			{
				token_t *stop = src + VEC_SIZE;
				while (src < stop)
                    replace_pair_step(&dst, &src, l, r, replacement);
				goto next;

			}
		}
		*(vtok_t*)dst = a;
		dst += VEC_SIZE;
		src += VEC_SIZE;
	next:;
	}

	while (src < end)
        replace_pair_step(&dst, &src, l, r, replacement);

	if (src == end)
    	*(dst++) = *src;

	return ((size_t)dst - (size_t)tokens) / sizeof(token_t);
}

