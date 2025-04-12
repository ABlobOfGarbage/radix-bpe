

#define USE_TIMER

#define MAX_TIMER_ENTRIES 16


typedef struct {
	const char *name;
	clock_t time;
} TimerEntry;

struct {
	int num_entries;
	TimerEntry entries[MAX_TIMER_ENTRIES];
	clock_t start;
	int num_iterations;
	int max_name_len;
} timer = {};


void timer_start()
{
#ifdef USE_TIMER
	timer.start = clock();
#endif // USE_TIMER
}

void timer_end(const char *name)
{
#ifdef USE_TIMER
	for (int i = 0; i < timer.num_entries; i++)
	{
    	if (0 == strcmp(timer.entries[i].name, name))
    	{
        	timer.entries[i].time += clock() - timer.start;
        	return;
    	}
	}
	fprintf(stderr, "Timer error: \"%s\" not entry.\n", name);
	exit(1);
#else
    (void)name;
#endif // USE_TIMER
}

void timer_add(const char *name)
{
#ifdef USE_TIMER
	assert(timer.num_entries < MAX_TIMER_ENTRIES);
	timer.entries[timer.num_entries++] = (TimerEntry) {
    	.name = name,
    	.time = 0,
	};
	int len = strlen(name);
	if (len > timer.max_name_len)
    	timer.max_name_len = len;
#else
    (void)name;
#endif // USE_TIMER
}

void timer_log_info()
{
#ifdef USE_TIMER
	assert(timer.num_iterations);
	float time_multiplier = 1000.0 / (CLOCKS_PER_SEC * timer.num_iterations);
	for (int i = 0; i < timer.num_entries; i++)
	{
		TimerEntry *entry = &timer.entries[i];
		log_printf("   %s: ", entry->name);
		int j = strlen(entry->name);
		while (j++ < timer.max_name_len)
    		log_printf(" ");
		log_printf("%f\n", entry->time * time_multiplier);
		entry->time = 0;
	}
#endif // USE_TIMER
}
