#include <err.h>
#include <string.h>
#include <strings.h>
#include <cairo.h>
#include <rsvg.h>
#include <rsvg-cairo.h>
#include <stdlib.h>

#include "neg_conf.h"
#include "neg_opts.h"
#include "neg_state.h"
#include "neg_rndr.h"

static const char *next_layer(const char *p)
{
	const char *comma = strchr(p, ',');
	if (!comma)
		return NULL;
	return comma+1;
}

int main(int argc, char *argv[])
{
	int argi;
	RsvgHandle *rsvg;
	GError *err;
	RsvgDimensionData rsvg_size;
	struct neg_conf conf;
	struct neg_render *rndr;
	static neg_render_ctx *ctx;

	neg_program = argv[0];

	neg_conf_init(&conf);
	argi = neg_parse_cmdline(&conf, argc, argv);
	conf.out.type = NEG_RNDR_MANY_PNGS;

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(conf.in.name, &err);
	if (!rsvg)
		errx(1, "Could not load file %s", conf.in.name);

	rsvg_handle_get_dimensions(rsvg, &rsvg_size);

	// {{{ TODO shoudl go into neg_set_defaults() or some such
	if (!conf.out.width)
		conf.out.width = rsvg_size.width;
	if (!conf.out.height)
		conf.out.height = rsvg_size.height;
	if (!conf.out.name)
		conf.out.name = "slide-%03u.%s";
	// }}}

	rndr = neg_get_renderer(conf.out.type);
	if (!rndr)
		errx(1, "Could not handler output type #%u", conf.out.type);

	printf("format %s\n", rndr->name);
	printf("input  %u x %u\n", rsvg_size.width, rsvg_size.height);
	printf("output %u x %u\n", (unsigned)conf.out.width, (unsigned)conf.out.height);

	ctx = rndr->init(&conf);

	for (; argi < argc; argi++) {
		cairo_surface_t* csurf;
		cairo_t *c;
		const char *p;

		csurf = rndr->slide_start(ctx);

		if (!csurf)
			errx(1, "Could not create %s surface", rndr->name);
		c = cairo_create(csurf);
		if (!c)
			errx(1, "Could not create cairo");

		cairo_scale(c, conf.out.width/rsvg_size.width,
				conf.out.height/rsvg_size.height);

		for (p = argv[argi]; p; p = next_layer(p)) {
			char id[1 + strcspn(p, ",") + 1];
			id[0] = '#';
			memcpy(id+1, p, strcspn(p, ","));
			id[1+strcspn(p, ",")] = '\0';

			printf(" - %s\n", id);
			rsvg_handle_render_cairo_sub(rsvg, c, id);
		}

		if (! rndr->slide_end(ctx))
			errx(1, "error writing out %s slide", rndr->name);

		cairo_destroy(c);
		cairo_surface_destroy(csurf);
	}
	rndr->exit(ctx);
	return 0;
}
