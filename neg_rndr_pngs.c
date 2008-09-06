#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "neg_rndr.h"
#include "neg_conf.h"
#include "neg_file_util.h"

struct neg_render_ctx {
	struct neg_conf *conf;

	// per slide info
	struct neg_filename fn; // tracks the filename
	cairo_surface_t *csurf; // current cairo surface
};

static neg_render_ctx neg_rndr_pngs_init(struct neg_conf *conf)
{
	struct neg_render_ctx *ctx;

	ctx = calloc(1, sizeof(*ctx));

	ctx->conf = conf;
	neg_filename_init(&ctx->fn, conf->out.name, "png");

	return ctx;
}

static cairo_surface_t* neg_rndr_pngs_slide_start(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	const char *fn;
	fn = neg_filename_next(&ctx->fn);
	printf("%s\n", fn);

	ctx->csurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			ctx->conf->out.width, ctx->conf->out.height);

	return ctx->csurf;
}

static bool neg_rndr_pngs_slide_end(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	cairo_status_t rc;
	const char *fn;
	fn = neg_filename_current(&ctx->fn);

	rc = cairo_surface_write_to_png(ctx->csurf, fn);

#if 0
	fprintf(stderr, "cairo_surface_write_to_png: %s\n",
			cairo_status_to_string(rc));
#endif

	return rc == CAIRO_STATUS_SUCCESS;
}

static bool neg_rndr_pngs_exit(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	neg_filename_exit(&ctx->fn);
	free(ctx);
	return true;
}

struct neg_render neg_rndr_pngs =
{
	.name = "pngs",

	.init        = neg_rndr_pngs_init,
	.slide_start = neg_rndr_pngs_slide_start,
	.slide_end   = neg_rndr_pngs_slide_end,
	.exit        = neg_rndr_pngs_exit,
};
