#ifndef __NEG_FILE_UTIL_H__
#define __NEG_FILE_UTIL_H__

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

struct neg_filename {
	const char *base;           // file name pattern, ex: "slides-###"
	size_t      base_len;       // length of pattern, ex: 14

	unsigned    index_ofs;      // offset of '###', ex: 7
	unsigned    index_len;      // length of '###', ex: 3

	char       *buffer;         // private buffer

	unsigned    index_max;      // highest index, ex:  999
	unsigned    index;          // current value
};

extern void neg_filename_init(struct neg_filename *fn, const char *base);

extern const char *neg_filename_next(struct neg_filename *fn);

static inline const char *neg_filename_current(struct neg_filename *fn)
{
	return fn->buffer;
}

extern void neg_filename_exit(struct neg_filename *fn);

#endif // __NEG_FILE_UTIL_H__
