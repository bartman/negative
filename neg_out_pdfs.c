#include <stdlib.h>
#include <string.h>

#include "neg_out.h"
#include "neg_conf.h"

static neg_output_ctx neg_out_pdfs_init(struct neg_conf *conf)
{
	return NULL;
}

static cairo_surface_t* neg_out_pdfs_slide_start(neg_output_ctx opaque)
{
	return NULL;
}

static bool neg_out_pdfs_slide_end(neg_output_ctx opaque)
{
	return false;
}

static bool neg_out_pdfs_exit(neg_output_ctx opaque)
{
	return false;
}

struct neg_output neg_out_pdfs =
{
	.name = "pdfs",
	.ext = "pdf",

	.init        = neg_out_pdfs_init,
	.slide_start = neg_out_pdfs_slide_start,
	.slide_end   = neg_out_pdfs_slide_end,
	.exit        = neg_out_pdfs_exit,
};
