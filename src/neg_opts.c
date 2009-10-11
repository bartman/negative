#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "neg_opts.h"

#define _GNU_SOURCE
#include <getopt.h>

static void dump_help(int rc, const char *argv0)
{
	printf("%s [-h] [-x <width>] [-y <height>] [-t <type>] [-o <output>] <input>...\n"
	       "\n"
	       "<type> is one of:\n"
	       "  pdf      - generate a single pdf\n"
	       "  pdfs     - generate a pdf per slide\n"
	       "  pngs     - generate a png per slide\n"
	       , argv0);
	exit(rc);
}

int neg_parse_cmdline(struct neg_conf *conf, int argc, char *argv[])
{
	int argi, max_names, i;

	if (argc < 2)
		errx(1, "Insufficient arguments, see %s -h.", argv[0]);

	for (argi = 1; argi < argc; argi++) {
		int cmd;

		// only support short arguments
		if (argv[argi][0] != '-' || argv[argi][2])
			break;

		cmd = argv[argi][1];

		switch (cmd) {
		case 'h':
			dump_help(0, argv[0]);
		case 'x':
			argi++;
			conf->out.width = atoi(argv[argi]);
			if (conf->out.width < 512 || conf->out.width > 10000)
				errx(1, "-x '%s' is invalid", argv[argi]);
			break;
		case 'y':
			argi++;
			conf->out.height = atoi(argv[argi]);
			if (conf->out.height < 512 || conf->out.height > 10000)
				errx(1, "-y '%s' is invalid", argv[argi]);
			break;
		case 't':
			argi++;
			conf->out.type = neg_get_render_type(argv[argi]);
			if ((unsigned)conf->out.type > NEG_RNDR_TYPE_MAX)
				errx(1, "-o '%s' is invalid", argv[argi]);
			break;
		case 'o':
			argi++;
			conf->out.name = argv[argi];
			break;
		default:
			errx(1, "Unhandled option %s", argv[argi-1]);
			break;
		}
	}

	max_names = argc - argi + 2;
	conf->in.names = calloc(max_names, sizeof(char*));
	if (!conf->in.names)
		errx(1, "Could not allocate buffer: %s",
				strerror(errno));

	for (i=0; argv[argi]; argi++, i++) {
		const char *name = argv[argi];
		struct stat st;
		int rc;

		rc = stat(name, &st);
		if (rc < 0)
			errx(1, "%s: %s", name,
					strerror(errno));

		conf->in.names[i] = name;
		conf->in.file_count ++;
	}

	return argi;
}
