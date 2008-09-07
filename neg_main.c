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
#include "neg_load_rsvg.h"

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
	struct neg_rsvg *rsvg;
	struct neg_conf conf;
	struct neg_render *rndr;
	static neg_render_ctx *ctx;

	neg_program = argv[0];

	neg_conf_init(&conf);
	argi = neg_parse_cmdline(&conf, argc, argv);

	rsvg = neg_rsvg_open(conf.in.name);

	// {{{ TODO shoudl go into neg_set_defaults() or some such
	if (!conf.out.width)
		conf.out.width = rsvg->size.width;
	if (!conf.out.height)
		conf.out.height = rsvg->size.height;
	if (!conf.out.name)
		conf.out.name = "slide-####";
	// }}}

	rndr = neg_get_renderer(conf.out.type);
	if (!rndr)
		errx(1, "Could not handler output type #%u", conf.out.type);

	printf("format %s\n", rndr->name);
	printf("input  %u x %u\n", rsvg->size.width, rsvg->size.height);
	printf("output %u x %u\n", (unsigned)conf.out.width,
			(unsigned)conf.out.height);

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

		cairo_scale(c, conf.out.width/rsvg->size.width,
				conf.out.height/rsvg->size.height);

		for (p = argv[argi]; p; p = next_layer(p)) {
			char id[1 + strcspn(p, ",") + 1];
			id[0] = '#';
			memcpy(id+1, p, strcspn(p, ","));
			id[1+strcspn(p, ",")] = '\0';

			printf(" - %s\n", id);
			rsvg_handle_render_cairo_sub(rsvg->handle, c, id);
		}

		if (! rndr->slide_end(ctx))
			errx(1, "error writing out %s slide", rndr->name);

		cairo_destroy(c);
		cairo_surface_destroy(csurf);
	}
	rndr->exit(ctx);
	return 0;
}
