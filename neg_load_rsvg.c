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

#include "neg_load_rsvg.h"

#define OVECCOUNT 30

#define MATCH_ID    "^[\t ]*id=\"(layer.*)\""
#define MATCH_LABEL "^[\t ]*inkscape:label=\"(.*)\""

static void build_layer_info(char *in, char *end)
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

		printf ("- %4u,%4u,%4u,%4u %9s...%s\n",
				ovector[0], ovector[1],
				ovector[2], ovector[3],
				id, label);

		free(label);
		free(id);

		p += ovector[1];
	}

	pcre_free(re_label);
	pcre_free(re_id);
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

RsvgHandle *neg_load_rsvg(const char *name)
{
	GError *err;
	RsvgHandle *rsvg;
	int fd_in;
	struct stat stat;
	int fd_tmp;
	char tempfilename[] = "/tmp/negative-XXXXXX";
	int rc;
	char *inmm, *end;

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

	build_layer_info(inmm, end);

	rewrite_display_inline(inmm, end, fd_tmp);
	close(fd_tmp);

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(tempfilename, &err);
	if (!rsvg)
		errx(1, "Could not load file %s", name);

	unlink(tempfilename);

	return rsvg;
}
