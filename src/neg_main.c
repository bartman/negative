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
#include "neg_rsvg.h"

int main(int argc, char *argv[])
{
	int i, j, rc;
	struct neg_rsvg *rsvg;
	struct neg_conf conf;
	struct neg_render *rndr;
	static neg_render_ctx *ctx;

	neg_program = argv[0];

	neg_conf_init(&conf);
	rc = neg_parse_cmdline(&conf, argc, argv);
	if (rc != argc)
		errx(1, "Garbage at end of cmdline, see %s -h.", neg_program);

	rsvg = neg_rsvg_open(conf.in.name);

	// {{{ TODO shoudl go into neg_set_defaults() or some such
	if (!conf.out.width)
		conf.out.width = rsvg->size.width;
	if (!conf.out.height)
		conf.out.height = rsvg->size.height;
	// }}}

	rndr = neg_get_renderer(conf.out.type);
	if (!rndr)
		errx(1, "Could not handler output type #%u", conf.out.type);

	printf("format %s\n", rndr->name);
	printf("input  %u x %u (%s)\n", rsvg->size.width, rsvg->size.height,
			conf.in.name);
	printf("output %u x %u (%s)\n", (unsigned)conf.out.width,
			(unsigned)conf.out.height, conf.out.name);

	ctx = rndr->init(&conf);

	for (i = rsvg->layer_count-1; i>=0; i--) {
		struct neg_layer *lyr = &rsvg->layers[i];
		cairo_t *c;

		if (lyr->flags & NEG_LAYER_HIDDEN)
			continue;

		c = rndr->slide_start(ctx);

		printf("  * %s : ", lyr->name);

		if (!c)
			errx(1, "Could not create cairo");

		cairo_scale(c, conf.out.width/rsvg->size.width,
				conf.out.height/rsvg->size.height);

		for(j=0; j<lyr->order->count; j++) {
			unsigned order = lyr->order->array[j];
			struct neg_layer *ord = &rsvg->layers[order];
			char id[strlen(ord->id)+2];

			printf("%s, ", ord->name);

			id[0] = '#';
			strcpy(id+1, ord->id);

			rsvg_handle_render_cairo_sub(rsvg->handle, c, id);
		}

		printf("\n");

		if (! rndr->slide_end(ctx))
			errx(1, "error writing out %s slide", rndr->name);
	}
	rndr->exit(ctx);
	neg_rsvg_close(rsvg);
	exit(0);
}
