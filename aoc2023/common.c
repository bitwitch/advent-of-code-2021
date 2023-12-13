#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <orca.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_COUNT(a) sizeof(a)/sizeof(*(a))

// char *chop_by_delimiter(char **str, char *delimiter) {
    // char *chopped = *str;

    // char *found = strstr(*str, delimiter);
    // if (found == NULL) {
        // *str += strlen(*str);
        // return chopped;
    // }

    // *found = '\0';
    // *str = found + strlen(delimiter);

    // return chopped;
// }

bool read_entire_file(char *filepath, char **out_data, u64 *out_size) {
	oc_file file = oc_file_open(OC_STR8(filepath), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
	if(oc_file_last_error(file) != OC_IO_OK) {
		return false;
	}

	*out_size = oc_file_size(file) + 1; // +1 to leave space for null terminator
	*out_data = malloc(*out_size);

	u64 bytes_read = oc_file_read(file, *out_size, *out_data);
	if (bytes_read != *out_size - 1) {
		oc_file_close(file);
		return false;
	}

	(*out_data)[bytes_read] = 0; // add null terminator
	oc_file_close(file);
	return true;
}

// ------------------------------------------------------------------------------
// PCG random number generator taken from https://en.wikipedia.org/wiki/Permuted_congruential_generator

static u64 pcg32_state      = 0x4d595df4d0f33173;   // Or something seed-dependent
static u64 const pcg32_multiplier = 6364136223846793005u;
static u64 const pcg32_increment  = 1442695040888963407u;	// Or an arbitrary odd constant

static u32 rotr32(u32 x, unsigned r) {
	return x >> r | x << (-r & 31);
}

u32 pcg32(void) {
	u64 x = pcg32_state;
	unsigned count = (unsigned)(x >> 59);   // 59 = 64 - 5

	pcg32_state = x * pcg32_multiplier + pcg32_increment;
	x ^= x >> 18;                           // 18 = (64 - 27)/2
	return rotr32((u32)(x >> 27), count);   // 27 = 32 - 5
}

void pcg32_init(u64 seed) {
	pcg32_state = seed + pcg32_increment;
	(void)pcg32();
}

u32 rand_range_u32(u32 min, u32 max) {
	u32 range = max - min + 1;
	return min + (pcg32() % range);
}

f32 rand_f32(void) {
	return (f32)((f64)pcg32() / (f64)UINT32_MAX);
}
