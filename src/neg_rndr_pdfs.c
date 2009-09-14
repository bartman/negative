#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <cairo-pdf.h>

#include "neg_rndr.h"
#include "neg_conf.h"
#include "neg_file_util.h"

struct neg_render_ctx {
	struct neg_conf *conf;

	// per slide info
	struct neg_filename fn; // tracks the filename
	cairo_surface_t *csurf; // current cairo surface
	cairo_t *cr;            // and the render context
};

static neg_render_ctx neg_rndr_pdfs_init(struct neg_conf *conf)
{
	struct neg_render_ctx *ctx;
	const char *base = "slide-####.pdf";

	if (conf->out.name)
		base = conf->out.name;

	ctx = calloc(1, sizeof(*ctx));

	ctx->conf = conf;
	neg_filename_init(&ctx->fn, base);

	return ctx;
}

static cairo_t* neg_rndr_pdfs_slide_start(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	const char *fn;
	fn = neg_filename_next(&ctx->fn);
	printf("%s\n", fn);

	ctx->csurf = cairo_pdf_surface_create(fn,
			ctx->conf->out.width, ctx->conf->out.height);
	if (!ctx->csurf)
		errx(1, "Could not create surface for %s.", fn);

	ctx->cr = cairo_create(ctx->csurf);

	return ctx->cr;
}

static bool neg_rndr_pdfs_slide_end(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	cairo_surface_flush(ctx->csurf);

	cairo_destroy(ctx->cr);
	cairo_surface_destroy(ctx->csurf);

	return true;
}

static bool neg_rndr_pdfs_exit(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	neg_filename_exit(&ctx->fn);
	free(ctx);
	return true;
}

struct neg_render neg_rndr_pdfs =
{
	.name = "pdfs",

	.init        = neg_rndr_pdfs_init,
	.slide_start = neg_rndr_pdfs_slide_start,
	.slide_end   = neg_rndr_pdfs_slide_end,
	.exit        = neg_rndr_pdfs_exit,
};
