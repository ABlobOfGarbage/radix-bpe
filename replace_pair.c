
size_t replace_pair(size_t length, token_t tokens[length], token_t l, token_t r, token_t replacement)
{
	token_t *dst = tokens;
	token_t *src = tokens;
	token_t *end = tokens + length - 1;

	while (src < end)
        replace_pair_step(&dst, &src, l, r, replacement);

	if (src == end)
    	*(dst++) = *src;

	return ((size_t)dst - (size_t)tokens) / sizeof(token_t);
}

