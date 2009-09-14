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
	char *fn;               // the filename
	cairo_surface_t *csurf; // current cairo surface
	cairo_t *cr;            // and the render context
};

static neg_render_ctx neg_rndr_1pdf_init(struct neg_conf *conf)
{
	struct neg_render_ctx *ctx;
	const char *base = "slides.pdf";
	int base_len, fn_len;

	if (conf->out.name)
		base = conf->out.name;

	base_len = strlen(base);
	fn_len = base_len + 1;

	ctx = calloc(1, sizeof(*ctx) + fn_len);

	ctx->conf = conf;
	ctx->fn   = (void*)(conf+1);

	strcpy(ctx->fn, base);

	printf("# %s\n", ctx->fn);

	ctx->csurf = cairo_pdf_surface_create(ctx->fn,
			ctx->conf->out.width, ctx->conf->out.height);
	if (!ctx->csurf)
		errx(1, "Could not create surface for %s.", ctx->fn);

	ctx->cr = cairo_create(ctx->csurf);

	return ctx;
}

static cairo_t* neg_rndr_1pdf_slide_start(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	return ctx->cr;
}

static bool neg_rndr_1pdf_slide_end(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	cairo_surface_flush(ctx->csurf);
	cairo_show_page (ctx->cr);

	return true;
}

static bool neg_rndr_1pdf_exit(neg_render_ctx opaque)
{
	struct neg_render_ctx *ctx = opaque;

	cairo_destroy(ctx->cr);
	cairo_surface_destroy(ctx->csurf);

	free(ctx);
	return true;
}

struct neg_render neg_rndr_1pdf =
{
	.name = "pdf",

	.init        = neg_rndr_1pdf_init,
	.slide_start = neg_rndr_1pdf_slide_start,
	.slide_end   = neg_rndr_1pdf_slide_end,
	.exit        = neg_rndr_1pdf_exit,
};
