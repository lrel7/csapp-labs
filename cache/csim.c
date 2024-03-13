#include "cachelab.h"
#include "getopt.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct Line {
    int valid;
    uint64_t tag;
    int timestamp;
} Line;

int s = -1, E = -1, b = -1;  // s: number of set index bits, E: number of lines per set, b: number of block bits
Line** cache;
int hits = 0, misses = 0, evictions = 0;  // hit, miss, and eviction counters
int verbose = 0;
int current_time = 0;

void init_cache() {
    int num_sets = (1 << s);
    cache = (Line**)calloc(num_sets, sizeof(Line*));
    for (int i = 0; i < num_sets; i++) {
        cache[i] = (Line*)calloc(E, sizeof(Line));
        for (int j = 0; j < E; j++) {  // init each line
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].timestamp = 0;
        }
    }
}

// parse the address to get the set index bits
uint64_t get_set(uint64_t addr) {
    return (addr >> b) & ((1 << s) - 1);
}

// parse the address to get the tag bits
uint64_t get_tag(uint64_t addr) {
    return addr >> (s + b);
}

// find the line according to the set and tag index
// return 1 if hit, 0 if miss
int get_line(uint64_t set, uint64_t tag) {
    Line* line;
    for (int i = 0; i < E; i++) {  // go through all the lines in the set
        line = &cache[set][i];
        if (line->valid && line->tag == tag) {  // the line contains valid info and matches the tag
            hits++;                           // increment hits count
            line->timestamp = current_time;    // update the timestamp
            if (verbose) {
                printf(" hit");
            }
            return 1;
        }
    }
    misses++;  // increment misses count
    if (verbose) {
        printf(" miss");
    }
    return 0;
}

// find an idle line in a set
// return 1 if found, 0 if not found
int get_idle_line(uint64_t set, uint64_t tag) {
    Line* line;
    for (int i = 0; i < E; i++) {  // go through all the lines in the set
        line = &cache[set][i];
        if (!line->valid) {
            line->valid = 1;                 // set the line to be valid
            line->tag = tag;                 // set the line's tag
            line->timestamp = current_time;  // update the timestamp
            return 1;
        }
    }
    return 0;
}

// find the least recently used line in a set to be evicted
void get_evicted_line(uint64_t set, uint64_t tag) {
    Line* lru_line = &cache[set][0];  // the least recently used line (with the least timestamp)
    for (int i = 1; i < E; i++) {     // go through all the lines in the set
        if (cache[set][i].timestamp < lru_line->timestamp) {
            lru_line = &cache[set][i];
        }
    }

    lru_line->tag = tag;                 // set the line's tag
    lru_line->timestamp = current_time;  // update the timestamp
    evictions++;                         // increment evictions count
    if (verbose) {
        printf(" eviction");
    }
}

void access_cache(uint64_t addr) {
    current_time++;  // increment the current time
    uint64_t set = get_set(addr);
    uint64_t tag = get_tag(addr);
    if (!(get_line(set, tag))) {           // miss
        if (!(get_idle_line(set, tag))) {  // no idle lines
            get_evicted_line(set, tag);    // evict the LRU line
        }
    }
}

int main(int argc, char* argv[]) {
    char op;
    char* tracefile = NULL;

    // parse arguments
    while ((op = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (op) {
            case 'h':
                printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                exit(1);
        }
    }

    // check if all necessary arguments are provided
    if (s == -1 || E == -1 || b == -1 || tracefile == NULL) {
        fprintf(stderr, "Error: Missing required arguments\n");
        exit(1);
    }

    // init the cache
    init_cache();

    // open the tracefile
    FILE* tfile = fopen(tracefile, "r");
    if (tfile == NULL) {
        fprintf(stderr, "Error: Unable to open tracefile\n");
        exit(1);
    }

    // read each line of the tracefile and possess them
    char inst;  // instruction: 'I', 'L', 'S', or 'M'
    uint64_t addr;
    int size;
    while (fscanf(tfile, " %c %lx,%d\n", &inst, &addr, &size) != -1) {
        if (inst == 'I') {
            continue;
        }

        if (verbose) {
            printf("%c %lx,%d", inst, addr, size);
        }

        access_cache(addr);
        if (inst == 'M') {
            access_cache(addr);  // access one more time for Modify instruction
        }

        if (verbose) {
            printf("\n");
        }
    }

    // close the tracefile
    fclose(tfile);

    printSummary(hits, misses, evictions);
    return 0;
}
