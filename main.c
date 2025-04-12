
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>


#define USE_COUNT1
//#define USE_SUM

#define DONT_VECTORIZE __attribute__((optimize("-fno-tree-vectorize")))

#define log_printf(...) fprintf(stderr, __VA_ARGS__)

#define ENABLE_VECTORIZATION

typedef uint32_t token_t;

#ifdef ENABLE_VECTORIZATION

#define VEC_SIZE 4

typedef token_t vtok_t __attribute__((vector_size (VEC_SIZE*sizeof(token_t)), aligned(__alignof(token_t))));

#endif // ENABLE_VECTORIZATION

typedef struct {
	token_t l, r;
	size_t count;
} PairDef;

typedef union {
	uint64_t u64;
	uint32_t u32[2];
	//uint16_t u16[4];
	uint8_t u8[8];
} Key;

struct {
	Key *a, *b;
	size_t i;
	size_t counts[256];
} rsb = {}; // radix sort buffers (kept global for convenience)

#define MAX_QUEUE_LEN 256
typedef struct {
	size_t num_defs;
	size_t min_valid_count;
	PairDef defs[MAX_QUEUE_LEN];
} Queue;


// used to see hom much time it takes to just
// iterate through the tokens array
//DONT_VECTORIZE
uint32_t sum(size_t length, token_t tokens[length])
{
	uint32_t x = 0;
	for (size_t i = 0; i < length; i++)
    	x += tokens[i];
	return x;
}


void radix_sort()
{
	Key *a = rsb.a;
	Key *b = rsb.b;

	for (int round = 0; round < 8; round++)
	{
		memset(rsb.counts, 0, sizeof(rsb.counts));

		for (size_t i = 0; i < rsb.i; i++)
    		rsb.counts[a[i].u8[round]]++;

		size_t index = 0;
		for (int i = 0; i < 256; i++)
		{
			size_t new_index = index + rsb.counts[i];
			rsb.counts[i] = index;
			index = new_index;
		}
		assert(index == rsb.i);

		for (size_t i = 0; i < rsb.i; i++)
		{
    		size_t *bucket = &rsb.counts[a[i].u8[round]];
			b[(*bucket)++] = a[i];
		}
		Key *c = a;
		a = b;
		b = c;
	}

	assert(a == rsb.a);
	assert(b == rsb.b);
}


void append_maxes(Queue *queue)
{
	size_t i = 0;
	while (i < rsb.i)
	{
		Key current = rsb.a[i];
		size_t count = 1;
		i++;
		while (i < rsb.i && rsb.a[i].u64 == current.u64)
		{
    		count++;
    		i++;
		}
		if (count < queue->min_valid_count)
    		continue;
		size_t j = queue->num_defs;
		while (j > 0 && queue->defs[j-1].count < count)
    		j--;
		if (j < MAX_QUEUE_LEN)
		{
			for (size_t k = queue->num_defs; k > j; k--)
			{
    			if (k == MAX_QUEUE_LEN)
        			continue;
    			queue->defs[k] = queue->defs[k-1];
			}
			queue->defs[j] = (PairDef) {
    			.l = current.u32[0],
    			.r = current.u32[1],
    			.count = count,
			};
			if (queue->num_defs < MAX_QUEUE_LEN)
    			queue->num_defs++;
		}
	}

	if (queue->num_defs == MAX_QUEUE_LEN)
    	queue->min_valid_count = queue->defs[MAX_QUEUE_LEN-1].count;
}

void queue_filter(Queue *queue, token_t l, token_t r)
{
	size_t i = 0;
	for (size_t j = 0; j < queue->num_defs; j++)
	{
		PairDef def = queue->defs[j];
    	if (def.l == r || def.r == l)
        	continue;
    	queue->defs[i++] = def;
	}
	queue->num_defs = i;
}

PairDef queue_pop(Queue *queue)
{
	assert(queue->num_defs > 0);
	PairDef max = queue->defs[0];
	for (size_t i = 1; i < queue->num_defs; i++)
		queue->defs[i-1] = queue->defs[i];
	queue->num_defs -= 1;
	return max;
}


void push_all_pairs(size_t length, token_t tokens[length])
{
	for (size_t i = 0; i < length-1; i++)
	{
		token_t l = tokens[i];
		token_t r = tokens[i+1];

		rsb.a[rsb.i++] = (Key) {.u32 = {l, r}};
		if (l == r)
		{
    		bool yes = false;
    		while (++i < length-1)
    		{
				l = tokens[i];
				r = tokens[i+1];
				if (l != r)
    				break;
				if (yes)
    				rsb.a[rsb.i++] = (Key) {.u32 = {l, r}};
				yes = !yes;
    		}
    		i--;
		}
	}
}


// this function is used by all push_filtered_pairs strategies
void push_filtered_pairs_step(size_t length, token_t tokens[length], token_t left, token_t right, token_t replacement, size_t *i)
{
	token_t l = tokens[*i];
	token_t r = tokens[*i+1];

	if (l == right || l == replacement || r == left || r == replacement)
    	rsb.a[rsb.i++] = (Key) {.u32 = {l, r}};
	if (l == r)
	{
		bool yes = false;
		while (++(*i) < length-1)
		{
			l = tokens[*i];
			r = tokens[*i+1];
			if (l != r)
				break;
			if (yes)
				rsb.a[rsb.i++] = (Key) {.u32 = {l, r}};
			yes = !yes;
		}
	}
	else
    	(*i)++;
}


// this function is used by all replace_pair strategies
void replace_pair_step(token_t **dst, token_t **src, token_t l, token_t r, token_t replacement)
{
	Key pair_key = {.u32 = {l, r}};
	if (*(uint64_t*)(*src) == pair_key.u64)
	{
		*((*dst)++) = replacement;
		(*src) += 2;
	}
	else
	{
		*((*dst)++) = *((*src)++);
	}
}


//#include "push_filtered_pairs.c"
//#include "push_filtered_pairs_look_ahead.c"
#ifdef ENABLE_VECTORIZATION
#include "push_filtered_pairs_vector.c"
#endif // ENABLE_VECTORIZATION

//#include "replace_pair.c"
#ifdef ENABLE_VECTORIZATION
//#include "replace_pair_vector.c"
#include "replace_pair_vector_look_ahead.c"
#endif // ENABLE_VECTORIZATION

#include "timer.c"


int main()
{
	FILE *f = fopen("input", "rb");
	assert(f);
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	assert(file_size >= 0);
	size_t length = file_size;
	rewind(f);

	token_t *tokens = malloc(sizeof(*tokens) * length);
	assert(tokens);
	for (size_t i = 0; i < length; i++)
	{
		int ch = getc(f);
		assert(0 <= ch && ch <= 255);
		tokens[i] = ch;
	}
	fclose(f);

	PairDef *defs = malloc(sizeof(*defs) * length);
	assert(defs);
	size_t num_defs = 0;

	rsb.a = malloc(sizeof(*rsb.a) * length);
	assert(rsb.a);
	rsb.b = malloc(sizeof(*rsb.b) * length);
	assert(rsb.b);

	Queue queue = {};

	clock_t start_time = clock();

	timer_add("push_all_pairs");
	timer_add("push_filtered_pairs");
	timer_add("replace_pair");
	timer_add("append_maxes");
	timer_add("radix_sort");
#ifdef USE_SUM
	timer_add("sum");
#endif // USE_SUM

	const int chunk_size = 1024;

	size_t begin_length = 0;
	PairDef begin_max = {};

	while (length > 1)
	{
		if (queue.num_defs == 0)
		{
    		rsb.i = 0;

			timer_start();
			push_all_pairs(length, tokens);
			timer_end("push_all_pairs");

			timer_start();
			radix_sort();
			timer_end("radix_sort");

			timer_start();
			queue.min_valid_count = 0;
			append_maxes(&queue);
			timer_end("append_maxes");

			assert(queue.num_defs > 0);
		}
		else
		{
    		rsb.i = 0;

			assert(num_defs > 0);
			PairDef prev_max = defs[num_defs-1];

    		timer_start();
    		push_filtered_pairs(length, tokens, prev_max.l, prev_max.r, num_defs - 1 + 256);
    		timer_end("push_filtered_pairs");

    		timer_start();
    		radix_sort();
    		timer_end("radix_sort");

    		timer_start();
    		append_maxes(&queue);
    		timer_end("append_maxes");

		}

#ifdef USE_SUM
		timer_start();
		// marked as volatile to prevent the compiler from removing the function call
		__attribute__((unused))
		volatile uint32_t _placeholder = sum(length, tokens);
		timer_end("sum");
#endif // USE_SUM

		PairDef max = queue_pop(&queue);


		if (timer.num_iterations == 0)
		{
			begin_length = length;
			begin_max = max;
		}

#ifdef USE_COUNT1
		if (max.count == 1)
		{
			log_printf("Starting the count=1 loop\n");
			token_t prev = tokens[0];
			for (size_t i = 1; i < length; i++)
			{
				defs[num_defs] = (PairDef) {
    				.l = prev,
    				.r = tokens[i],
    				.count = 1,
				};
				prev = num_defs + 256;
				num_defs++;
			}
			log_printf("Ending the count=1 loop\n");
			break;
		}
#endif // USE_COUNT1

		token_t replacement = num_defs + 256;
		defs[num_defs++] = max;

		queue_filter(&queue, max.l, max.r);

		timer_start();
		length = replace_pair(length, tokens, max.l, max.r, replacement);
		timer_end("replace_pair");


		//fprintf(stderr, "Identified <%u> : <%u> <%u> #%zu\n", replacement, max.l, max.r, max.count);

		timer.num_iterations++;
		if (timer.num_iterations >= chunk_size)
		{
			log_printf("length: %zu-%zu, max.count: %zu-%zu\n", begin_length, length, begin_max.count, max.count);
			timer_log_info();
			timer.num_iterations = 0;
		}
	}

	float whole_run_time = (clock() - start_time) / (float)CLOCKS_PER_SEC;
	log_printf("total run time: %f sec (%f min)\n", whole_run_time, whole_run_time / 60);

	FILE *out = fopen("out", "wb");
	assert(out);
	for (size_t i = 0; i < num_defs; i++)
	{
    	assert(1 == fwrite(&defs[i].l, sizeof(token_t), 1, out));
    	assert(1 == fwrite(&defs[i].r, sizeof(token_t), 1, out));
	}
	fclose(out);

	FILE *counts = fopen("counts", "wb");
	assert(counts);
	for (size_t i = 0; i < num_defs; i++)
	{
    	assert(1 == fwrite(&defs[i].count, sizeof(token_t), 1, counts));
	}
	fclose(counts);

	return 0;
}
