#include <err.h>
#include <string.h>
#include <strings.h>
#include <cairo.h>
#include <rsvg.h>
#include <rsvg-cairo.h>
#include <stdlib.h>
#include <libxml/parser.h>

#include "neg_conf.h"
#include "neg_opts.h"
#include "neg_state.h"
#include "neg_rndr.h"
#include "neg_rsvg.h"

struct neg_loop_vars {
	struct neg_conf       *conf;
	struct neg_render     *rndr;
	struct neg_render_ctx *ctx;
};

static void process_rsvg(struct neg_loop_vars *loop, const char *fname)
{
	int i, j;
	struct neg_rsvg *rsvg;

	rsvg = neg_rsvg_open(fname);

	// {{{ TODO shoudl go into neg_set_defaults() or some such
	if (!loop->conf->out.width)
		loop->conf->out.width = rsvg->size.width;
	if (!loop->conf->out.height)
		loop->conf->out.height = rsvg->size.height;
	// }}}

	printf("input  %u x %u (%s)\n", rsvg->size.width, rsvg->size.height,
			fname);
	printf("output %u x %u (%s)\n", (unsigned)loop->conf->out.width,
			(unsigned)loop->conf->out.height, loop->conf->out.name);

	if (! loop->ctx)
		loop->ctx = loop->rndr->init(loop->conf);

	for (i = rsvg->layer_count-1; i>=0; i--) {
		struct neg_layer *lyr = &rsvg->layers[i];
		cairo_t *c;

		if (lyr->flags & NEG_LAYER_HIDDEN)
			continue;

		c = loop->rndr->slide_start(loop->ctx);

		printf("  * %s : ", lyr->name);

		if (!c)
			errx(1, "Could not create cairo");

		cairo_scale(c, loop->conf->out.width/rsvg->size.width,
				loop->conf->out.height/rsvg->size.height);

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

		if (! loop->rndr->slide_end(loop->ctx))
			errx(1, "error writing out %s slide", loop->rndr->name);
	}
	neg_rsvg_close(rsvg);
}

int main(int argc, char *argv[])
{
	int i, rc;
	struct neg_conf conf;
	struct neg_loop_vars loop;

	memset(&loop, 0, sizeof(loop));
	loop.conf = &conf;

	neg_program = argv[0];

	neg_conf_init(&conf);
	rc = neg_parse_cmdline(&conf, argc, argv);
	if (rc != argc)
		errx(1, "Garbage at end of cmdline, see %s -h.", neg_program);

	loop.rndr = neg_get_renderer(conf.out.type);
	if (!loop.rndr)
		errx(1, "Could not handler output type #%u", conf.out.type);

	printf("format %s\n", loop.rndr->name);

        // Start up and cleanup the libxml2 parser here
        xmlInitParser();
	for (i=0; i<conf.in.file_count; i++)
		process_rsvg(&loop, conf.in.names[i]);
        xmlCleanupParser();

	if (loop.rndr && loop.ctx)
		loop.rndr->exit(loop.ctx);

        // Detect libxml2 memory leaks here
        xmlMemoryDump();
	exit(0);
}
