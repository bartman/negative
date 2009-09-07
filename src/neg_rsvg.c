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

#define MATCH_ID_OR_LABEL "^[\t ]*("                        \
				"inkscape:groupmode=\"layer\"\\s*" \
				"(id)=\"([^\"]*)\""        \
				"|"                         \
				"(inkscape:label)=\"([^\"]*)\"" \
				")"
enum match_ovector_offsets {
	MATCH_LINE_START,       // everything from <tab> on
	MATCH_LINE_END,

	MATCH_TEXT_START,       // just text in (...|...)
	MATCH_TEXT_END,

	MATCH_ID_STR_START,     // string "id" or NULL
	MATCH_ID_STR_END,
	MATCH_ID_VAL_START,     // value of id or NULL
	MATCH_ID_VAL_END,

	MATCH_LABEL_STR_START,  // string "inkscape:label" or NULL
	MATCH_LABEL_STR_END,
	MATCH_LABEL_VAL_START,  // value of inkscape:label or NULL
	MATCH_LABEL_VAL_END,
};

static void add_layer(struct neg_rsvg *rsvg, const char *id, const char *label)
{
	int index;
	struct neg_layer *lyr;

	index = rsvg->layer_count ++;

	if (!rsvg->layers || index >= rsvg->layer_size) {

		rsvg->layer_size += 10; // at least 10 more entries
		rsvg->layer_size *= 2;  // at least 2x the previous size

		rsvg->layers = realloc(rsvg->layers,
				rsvg->layer_size * sizeof(rsvg->layers[0]));
		if (!rsvg)
			errx(1, "Could not allocate buffer: %s",
					strerror(errno));
	}

	lyr = &rsvg->layers[index];

	memset(lyr, 0, sizeof(*lyr));

	rsvg->layers[index].id = id;
	rsvg->layers[index].label = label;
}

static void build_layer_info(struct neg_rsvg *rsvg, char *in, char *end)
{
	char *p;
	pcre *re;
	const char *error;
	int erroffset;
	int rc;
	int ovector[OVECCOUNT] = {0,};
	char *id = NULL, *label = NULL;

	re = pcre_compile(MATCH_ID_OR_LABEL, PCRE_MULTILINE,
			&error, &erroffset, NULL);
	if (!re)
		errx(1, "Internal regular expression error at %u: %s.",
				erroffset, error);

	for (p=in; p<end; ) {

		// find something

		rc = pcre_exec(re, NULL, p, end-p, 0, 0,
				ovector, OVECCOUNT);
		if (rc == PCRE_ERROR_NOMATCH)
			break;
		if (rc <= 0)
			errx(1, "Regular expression error %d.", rc);

		if ((ovector[MATCH_ID_STR_START]
					!= ovector[MATCH_ID_STR_END])
				&& (ovector[MATCH_ID_VAL_START]
					!= ovector[MATCH_ID_VAL_END])) {
			// matched an id

			if (id)
				errx(1, "Matched two 'id' fields in a row: %s.",
						id);

			id = strndup(p + ovector[MATCH_ID_VAL_START],
					ovector[MATCH_ID_VAL_END]
					- ovector[MATCH_ID_VAL_START]);

		} else if ((ovector[MATCH_LABEL_STR_START]
					!= ovector[MATCH_LABEL_STR_END])
				&& (ovector[MATCH_LABEL_VAL_START]
					!= ovector[MATCH_LABEL_VAL_END])) {
			// matched an inkscape:label

			if (label)
				errx(1, "Matched two 'inkscape:label' fields "
						"in a row: %s", label);

			label = strndup(p + ovector[MATCH_LABEL_VAL_START],
					ovector[MATCH_LABEL_VAL_END]
					- ovector[MATCH_LABEL_VAL_START]);

		} else {
			char *tmp;

			tmp = strndup(p + ovector[MATCH_TEXT_START],
					ovector[MATCH_TEXT_END]
					- ovector[MATCH_TEXT_START]);

			errx(1, "Cannot handle regexp match: %s.", tmp);
		}

		if (id && label) {
#if 0
			printf ("- %9s...%s\n",
					id, label);
#endif

			add_layer(rsvg, id, label);

			id = label = NULL;
		}

		p += ovector[1];
	}

	pcre_free(re);
}

static inline int find_largest_substring_match(const struct neg_rsvg *rsvg,
		const struct neg_layer *lyr)
{
	int i;
	int best_len = -1;
	int best_idx = -1;

	for(i=0; i<rsvg->layer_count; i++) {
		struct neg_layer *sub = &rsvg->layers[i];

		if (sub->flags & NEG_LAYER_STICKY)
			continue;

		if (lyr == sub)
			continue;

		if (sub->name_len >= lyr->name_len
				|| strncmp(lyr->name, sub->name,
					sub->name_len))
			continue;

		// found a sub layer that has a name that's a prefix
		// of our layer

		if (best_len < (signed)sub->name_len) {
			best_len = sub->name_len;
			best_idx = i;
		}
	}

	return best_idx;
}

static void resolve_layer_order(struct neg_rsvg *rsvg, int index)
{
	struct neg_layer *lyr = &rsvg->layers[index];
	int j;

	if (lyr->flags & NEG_LAYER_RESOLVED)
		return;

	if (lyr->flags & NEG_LAYER_RESOLVING || lyr->local)
		errx(1, "Recursion discovered in layer resolution "
				"@%s label='%s' %04x %p %u",
				lyr->id, lyr->label,
				lyr->flags, lyr->local,
				lyr->local ? lyr->local->count : 0);

	lyr->local = neg_order_new(rsvg->layer_count);
	lyr->order = neg_order_new(rsvg->layer_count);

	// sticky tags will be included later
	if (lyr->flags & NEG_LAYER_STICKY)
		goto done;

	lyr->flags |= NEG_LAYER_RESOLVING;

	// find and process any substring matches
	j = find_largest_substring_match(rsvg, lyr);
	if (j>=0) {
		struct neg_layer *sub = &rsvg->layers[j];

		resolve_layer_order(rsvg, j);

		if (lyr->name[sub->name_len] == '^') {
			// this index will be above sub

			neg_order_append(lyr->local, sub->local);
			neg_order_add(lyr->local, index);
		} else {
			// this index will be below sub
			neg_order_add(lyr->local, index);
			neg_order_append(lyr->local, sub->local);
		}
	} else
		// there is no substring match
		neg_order_add(lyr->local, index);

	// finished

done:
	neg_order_copy(lyr->order, rsvg->sticky_below);
	neg_order_append(lyr->order, lyr->local);
	neg_order_append(lyr->order, rsvg->sticky_above);

	lyr->flags &= ~NEG_LAYER_RESOLVING;
	lyr->flags |= NEG_LAYER_RESOLVED;
}

static void process_layer_info(struct neg_rsvg *rsvg)
{
	int i;
	const char *p;

	// allocate arrays for sticky indeces
	rsvg->sticky_above = neg_order_new(rsvg->layer_count);
	rsvg->sticky_below = neg_order_new(rsvg->layer_count);

	for (i=0; i<rsvg->layer_count; i++) {
		struct neg_layer *lyr = &rsvg->layers[i];

		lyr->flags = 0;

		p = lyr->label;
		while(*p) {
			if (*p == '#') {
				lyr->flags |= NEG_LAYER_HIDDEN;
			} else if (*p == '^') {
				lyr->flags |= NEG_LAYER_STICKY_ABOVE;
				neg_order_add(rsvg->sticky_above, i);
			} else if (*p == '_') {
				lyr->flags |= NEG_LAYER_STICKY_BELOW;
				neg_order_add(rsvg->sticky_below, i);
			} else
				break;
			p++;
		}
		lyr->name = strdup(p);
		lyr->name_len = strlen(lyr->name);
	}

	for (i=0; i<rsvg->layer_count; i++)
		resolve_layer_order(rsvg, i);

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
		struct neg_layer *lyr = &rsvg->layers[i];
		free((void*)lyr->id);
		free((void*)lyr->label);
		free((void*)lyr->name);
		neg_order_free(lyr->local);
		neg_order_free(lyr->order);
	}
	neg_order_free(rsvg->sticky_above);
	neg_order_free(rsvg->sticky_below);
	free(rsvg->layers);
	free(rsvg);
}

struct neg_order *neg_order_new(unsigned size)
{
	struct neg_order *order;

	order = malloc(sizeof(*order)
			+ sizeof(order->array[0]) * size);
	if (!order)
		errx(1, "Could not allocate buffer: %s",
				strerror(errno));

	order->size = size;
	order->count = 0;

	return order;
}

void neg_order_free(struct neg_order *order)
{
	free(order);
}

void neg_order_add(struct neg_order *order, unsigned n)
{
	// TODO: range check
	order->array[ order->count ++ ] = n;
}

void neg_order_copy(struct neg_order *dest, const struct neg_order *src)
{
	// TODO: range check
	dest->count = 0;
	neg_order_append(dest, src);
}

void neg_order_append(struct neg_order *dest, const struct neg_order *src)
{
	// TODO: range check
	memcpy(dest->array + dest->count,
			src->array,
			sizeof(dest->array[0]) * src->count);
	dest->count += src->count;
}

