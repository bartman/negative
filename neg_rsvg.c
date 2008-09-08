#define _GNU_SOURCE
#include <err.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <pcre.h>

#include "neg_rsvg.h"

#define OVECCOUNT 30

#define MATCH_ID    "^[\t ]*id=\"(layer.*)\""
#define MATCH_LABEL "^[\t ]*inkscape:label=\"(.*)\""

static void add_layer(struct neg_rsvg *rsvg, const char *id, const char *label)
{
	int index;

	index = rsvg->layer_count ++;

	if (index >= rsvg->layer_size) {

		rsvg->layer_size += 10; // at least 10 more entries 
		rsvg->layer_size *= 2;  // at least 2x the previous size

		rsvg->layers = realloc(rsvg->layers,
				rsvg->layer_size * sizeof(rsvg->layers[0]));
		if (!rsvg)
			errx(1, "Could not allocate buffer: %s",
					strerror(errno));
	}

	rsvg->layers[index].id = id;
	rsvg->layers[index].label = label;
}

static void build_layer_info(struct neg_rsvg *rsvg, char *in, char *end)
{
	char *p;
	pcre *re_id, *re_label;
	const char *error;
	int erroffset;
	int rc;
	int ovector[OVECCOUNT] = {0,};

	re_id = pcre_compile(MATCH_ID, PCRE_MULTILINE,
			&error, &erroffset, NULL);
	if (!re_id)
		errx(1, "Internal regular expression error at %u: %s.",
				erroffset, error);

	re_label = pcre_compile(MATCH_LABEL, PCRE_MULTILINE,
			&error, &erroffset, NULL);
	if (!re_id)
		errx(1, "Internal regular expression error at %u: %s.",
				erroffset, error);

	for (p=in; p<end; ) {

		char *id, *label;

		// find id

		rc = pcre_exec(re_id, NULL, p, end-p, 0, 0,
				ovector, OVECCOUNT);
		if (rc == PCRE_ERROR_NOMATCH)
			break;
		if (rc <= 0)
			errx(1, "Regular expression error %d.", rc);

		id = strndup(p + ovector[2], ovector[3] - ovector[2]);

		p += ovector[1];

		// find label

		rc = pcre_exec(re_label, NULL, p, end-p, 0, 0,
				ovector, OVECCOUNT);
		if (rc == PCRE_ERROR_NOMATCH)
			break;
		if (rc <= 0)
			errx(1, "Regular expression error %d.", rc);

		label = strndup(p + ovector[2], ovector[3] - ovector[2]);

		// dump

#if 0
		printf ("- %4u,%4u,%4u,%4u %9s...%s\n",
				ovector[0], ovector[1],
				ovector[2], ovector[3],
				id, label);
#endif

		add_layer(rsvg, id, label);

		p += ovector[1];
	}

	pcre_free(re_label);
	pcre_free(re_id);
}

static void process_layer_info(struct neg_rsvg *rsvg)
{
	int i, j;
	const char *p;
	unsigned *always_above, always_above_count;
	unsigned *always_below, always_below_count;

	// allocate arrays for sticky indeces
	always_above = calloc(rsvg->layer_count, sizeof(always_above[0]));
	always_below = calloc(rsvg->layer_count, sizeof(always_below[0]));
	always_above_count = always_below_count = 0;

	for (i=0; i<rsvg->layer_count; i++) {
		struct neg_layer *lyr = &rsvg->layers[i];

		lyr->flags = 0;

		p = lyr->label;
		while(*p) {
			if (*p == '#') {
				lyr->flags |= NEG_LAYER_HIDDEN;
			} else if (*p == '^') {
				lyr->flags |= NEG_LAYER_STICKY_ABOVE;
				always_above[always_above_count++] = i;
			} else if (*p == '_') {
				lyr->flags |= NEG_LAYER_STICKY_BELOW;
				always_below[always_below_count++] = i;
			} else
				break;
			p++;
		}
		lyr->name = strdup(p);
		lyr->name_len = strlen(lyr->name);
	}

	for (i=0; i<rsvg->layer_count; i++) {
		struct neg_layer *lyr = &rsvg->layers[i];

		if (lyr->flags & NEG_LAYER_HIDDEN)
			continue;

		lyr->order = calloc(rsvg->layer_count, sizeof(lyr->order[0]));
		memcpy(lyr->order, always_below,
				sizeof(lyr->order[0]) * always_below_count);
		lyr->order_count = always_below_count;

		lyr->order[lyr->order_count++] = i;

		for(j=0; j<rsvg->layer_count; j++) {
			struct neg_layer *sub = &rsvg->layers[j];

			if (i == j)
				continue;

			if (sub->name_len > lyr->name_len
					|| strncmp(lyr->name, sub->name,
						sub->name_len))
				continue;

			// found a sub layer that has a name that's a prefix
			// of our layer

			lyr->order[lyr->order_count++] = j;
		}

		memcpy(&lyr->order[lyr->order_count], always_above,
				sizeof(lyr->order[0]) * always_above_count);
		lyr->order_count += always_above_count;
	}

#if 0
	for (i=rsvg->layer_count-1; i>=0; i--) {
		struct neg_layer *lyr = &rsvg->layers[i];

		if (lyr->flags & NEG_LAYER_HIDDEN)
			continue;

		printf("- %s : ", lyr->name);

		for(j=0; j<lyr->order_count; j++) {
			int order = lyr->order[j];
			struct neg_layer *ord = &rsvg->layers[order];

			printf("%s, ", ord->name);
		}

		printf("\n");
	}
#endif

	free (always_below);
	free (always_above);
}

#define DISPLAY_NONE "display:none"
#define DISPLAY_INLINE "display:inline"

static void rewrite_display_inline(char *in, char *end, int fd_out)
{
	char *p;
	pcre *re;
	const char *error;
	int erroffset;
	int rc;
	int ovector[OVECCOUNT];

	re = pcre_compile(DISPLAY_NONE, 0, &error, &erroffset, NULL);
	if (!re)
		errx(1, "Internal regular expression error at %u: %s.",
				erroffset, error);

	for (p=in; p<end; ) {

		rc = pcre_exec(re, NULL, p, end-p, 0, 0, ovector, OVECCOUNT);
		if (rc == PCRE_ERROR_NOMATCH)
			break;
		if (rc <= 0)
			errx(1, "Regular expression error %d.", rc);

		write(fd_out, p, ovector[0]);
		write(fd_out, DISPLAY_INLINE, strlen(DISPLAY_INLINE));

		p += ovector[0] + strlen(DISPLAY_NONE);
	}

	pcre_free(re);

	write(fd_out, p, end-p);
}

struct neg_rsvg *neg_rsvg_open(const char *name)
{
	GError *err;
	struct neg_rsvg *rsvg;
	int fd_in;
	struct stat stat;
	int fd_tmp;
	char tempfilename[] = "/tmp/negative-XXXXXX";
	int rc;
	char *inmm, *end;

	rsvg = calloc(1, sizeof(*rsvg));
	if (!rsvg)
		errx(1, "Could not allocate buffer: %s",
				strerror(errno));

	fd_in = open(name, O_RDONLY);
	if (fd_in < 0)
		errx(1, "Could not open input file %s: %s",
				name, strerror(errno));

	rc = fstat(fd_in, &stat);
	if (rc < 0)
		errx(1, "Could not stat input file %s: %s",
				name, strerror(errno));

	inmm = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd_in, 0); 
	if (!inmm)
		errx(1, "Could not mmap input file %s: %s",
				name, strerror(errno));

	end = inmm + stat.st_size;

	fd_tmp = mkstemp(tempfilename);
	if (fd_tmp < 0)
		errx(1, "Could not create a temporary file in /tmp/: %s",
				strerror(errno));

	build_layer_info(rsvg, inmm, end);

	process_layer_info(rsvg);

	rewrite_display_inline(inmm, end, fd_tmp);
	close(fd_tmp);

	rsvg_init();
	rsvg->handle = rsvg_handle_new_from_file(tempfilename, &err);
	if (!rsvg->handle)
		errx(1, "Could not load file %s", name);

	unlink(tempfilename);

	rsvg_handle_get_dimensions(rsvg->handle, &rsvg->size);

	return rsvg;
}

void neg_rsvg_close(struct neg_rsvg *rsvg)
{
	int i;

	for (i=0; i<rsvg->layer_count; i++) {
		free((void*)rsvg->layers[i].id);
		free((void*)rsvg->layers[i].label);
		free((void*)rsvg->layers[i].name);
	}
	free(rsvg->layers);
	free(rsvg);
}
