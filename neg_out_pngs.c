#include <stdlib.h>
#include <string.h>

#include "neg_out.h"

static neg_output_ctx neg_out_pngs_init(struct neg_conf *conf)
{
	return NULL;
}

static cairo_surface_t* neg_out_pngs_slide_start(neg_output_ctx opaque)
{
	return NULL;
}

static void neg_out_pngs_slide_end(neg_output_ctx opaque)
{
	return;
}

static void neg_out_pngs_exit(neg_output_ctx opaque)
{
	return;
}

struct neg_output neg_out_pngs =
{
	.name = "pngs",
	.ext = "png",

	.init        = neg_out_pngs_init,
	.slide_start = neg_out_pngs_slide_start,
	.slide_end   = neg_out_pngs_slide_end,
	.exit        = neg_out_pngs_exit,
};
