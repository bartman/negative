#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <cairo-pdf.h>

#include "neg_out.h"
#include "neg_conf.h"

struct neg_output_ctx {
	struct neg_conf *conf;

	// per slide info
	int slide;               // which slide we are on
	unsigned fnsize;         // size of filename
	char *filename;          // the current filename
	cairo_surface_t *csurf;  // the current cairo surface
};

static neg_output_ctx neg_out_pdfs_init(struct neg_conf *conf)
{
	struct neg_output_ctx *ctx;
	unsigned fnsize;

	fnsize = strlen(conf->out.name)
		+ 9   // digits in the largest '%u' that we will see
		+ 3   // the extenssion
		+ 10; // null termination and fluff

	ctx = calloc(1, sizeof(*ctx) + fnsize);

	ctx->conf = conf;
	ctx->fnsize = fnsize;
	ctx->filename = (void*)(ctx+1);

	return ctx;
}

static cairo_surface_t* neg_out_pdfs_slide_start(neg_output_ctx opaque)
{
	struct neg_output_ctx *ctx = opaque;

	snprintf(ctx->filename, ctx->fnsize, ctx->conf->out.name,
			++ctx->slide, "pdf");
	printf("%s\n", ctx->filename);

	ctx->csurf = cairo_pdf_surface_create(ctx->filename,
			ctx->conf->out.width, ctx->conf->out.height);

	return ctx->csurf;
}

static bool neg_out_pdfs_slide_end(neg_output_ctx opaque)
{
	struct neg_output_ctx *ctx = opaque;

	cairo_surface_flush(ctx->csurf);

	return true;
}

static bool neg_out_pdfs_exit(neg_output_ctx opaque)
{
	struct neg_output_ctx *ctx = opaque;

	free(ctx);
	return true;
}

struct neg_output neg_out_pdfs =
{
	.name = "pdfs",

	.init        = neg_out_pdfs_init,
	.slide_start = neg_out_pdfs_slide_start,
	.slide_end   = neg_out_pdfs_slide_end,
	.exit        = neg_out_pdfs_exit,
};
