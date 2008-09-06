#include <stdlib.h>
#include <string.h>

#include "neg_rndr.h"
#include "neg_conf.h"

static neg_render_ctx neg_rndr_1pdf_init(struct neg_conf *conf)
{
	return NULL;
}

static cairo_surface_t* neg_rndr_1pdf_slide_start(neg_render_ctx opaque)
{
	return NULL;
}

static bool neg_rndr_1pdf_slide_end(neg_render_ctx opaque)
{
	return false;
}

static bool neg_rndr_1pdf_exit(neg_render_ctx opaque)
{
	return false;
}

struct neg_render neg_rndr_1pdf =
{
	.name = "1pdf",

	.init        = neg_rndr_1pdf_init,
	.slide_start = neg_rndr_1pdf_slide_start,
	.slide_end   = neg_rndr_1pdf_slide_end,
	.exit        = neg_rndr_1pdf_exit,
};
