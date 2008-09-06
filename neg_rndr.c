#include <stdlib.h>
#include <string.h>

#include "neg_rndr.h"

struct neg_render *neg_render_table;

extern struct neg_render neg_rndr_1pdf;
extern struct neg_render neg_rndr_pdfs;
extern struct neg_render neg_rndr_pngs;

static struct neg_render *handlers[NEG_RNDR_TYPE_MAX+1] = {
	[NEG_RNDR_SINGLE_PDF] = &neg_rndr_1pdf,
	[NEG_RNDR_MANY_PDFS]  = &neg_rndr_pdfs,
	[NEG_RNDR_MANY_PNGS]  = &neg_rndr_pngs,
	[NEG_RNDR_TYPE_MAX]   = NULL,
};

enum neg_render_type neg_get_render_type(const char *name)
{
	enum neg_render_type type;

	for (type = 0; type < NEG_RNDR_TYPE_MAX; type++) {
		struct neg_render *out;

		out = handlers[type];

		if (!out || !out->name || 
				strcasecmp(out->name, name))
			continue;

		break;
	}

	return type;
}

struct neg_render *neg_get_renderer(enum neg_render_type type)
{
	struct neg_render *out = NULL;
	if (type < NEG_RNDR_TYPE_MAX)
		out = handlers[type];
	return out;
}
