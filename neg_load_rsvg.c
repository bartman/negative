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

#define DISPLAY_NONE "display:none"
#define DISPLAY_INLINE "display:inline"


RsvgHandle *neg_load_rsvg(const char *name)
{
	GError *err;
	RsvgHandle *rsvg;
	int fd_in;
	struct stat stat;
	int fd_tmp;
	char tempfilename[] = "/tmp/negative-XXXXXX";
	int rc;
	char *inmm, *p, *end;
	pcre *re;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];

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

	re = pcre_compile(DISPLAY_NONE, 0, &error, &erroffset, NULL);
	if (!re)
		errx(1, "Internal regular expression error at %u: %s.",
				erroffset, error);

	for (p=inmm; p<end; ) {

		rc = pcre_exec(re, NULL, p, end-p, 0, 0, ovector, OVECCOUNT);
		if (rc == PCRE_ERROR_NOMATCH)
			break;
		if (rc <= 0)
			errx(1, "Regular expression error %d.", rc);

		write(fd_tmp, p, ovector[0]);
		write(fd_tmp, DISPLAY_INLINE, strlen(DISPLAY_INLINE));

		p += ovector[0] + strlen(DISPLAY_NONE);
	}

	pcre_free(re);

	write(fd_tmp, p, end-p);
	close(fd_tmp);

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(tempfilename, &err);
	if (!rsvg)
		errx(1, "Could not load file %s", name);

	unlink(tempfilename);

	return rsvg;
}
