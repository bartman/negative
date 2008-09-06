#include <stdio.h>
#include <err.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <stdlib.h>

#include "neg_file_util.h"

void neg_filename_init(struct neg_filename *fn, const char *base,
		const char *ext)
{
	const char *p;
	size_t asize;

	memset(fn, 0, sizeof(*fn));

	fn->base = base;
	fn->base_len = strlen(base);

	fn->ext = ext;
	fn->ext_len = strlen(ext);

	// find the ### pattern
	p = index(base, '#');
	if (!p)
		errx(1, "Could not find a ### pattern in '%s'", base);
	
	fn->index_ofs = p - base;

	while (*(++p) == '#');

	fn->index_len = p - base - fn->index_ofs;

	asize = fn->base_len
		+ 1            // period
		+ fn->ext_len  // extension
		+1;            // 0-termination
	fn->buffer = calloc(1, asize);
	if (!fn->buffer)
		errx(1, "Memory allocation failure of a %u byte string.",
				asize);

	memcpy(fn->buffer, base, fn->base_len);
	fn->buffer[fn->base_len] = '.';
	memcpy(fn->buffer + fn->base_len + 1, ext, fn->ext_len);
	fn->buffer[fn->base_len + 1 + fn->ext_len] = 0;

	fn->index_max = (unsigned)pow(10,fn->index_len) - 1;
	fn->index = 1;
}

const char *neg_filename_next(struct neg_filename *fn)
{
	int rc;

	if (fn->index > fn->index_max)
		errx(1, "Exhausted indices that can be fit into '%s', at %u.",
				fn->base, fn->index);

	rc = snprintf(fn->buffer + fn->index_ofs, fn->index_len, "%0*u",
			fn->index_len, fn->index);
	if (rc != 1)
		errx(1, "Failed to generate file name for '%s' at index '%u'",
				fn->base, fn->index);

	return fn->buffer;
}

void neg_filename_exit(struct neg_filename *fn)
{
	free(fn->buffer);
}

