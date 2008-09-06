#include <stdlib.h>
#include <string.h>

#include "neg_out.h"

struct neg_output *neg_output_table;

extern struct neg_output neg_out_1pdf;
extern struct neg_output neg_out_pdfs;
extern struct neg_output neg_out_pngs;

static struct neg_output *handlers[NEG_OUT_TYPE_MAX+1] = {
	[NEG_OUT_SINGLE_PDF] = &neg_out_1pdf,
	[NEG_OUT_MANY_PDFS]  = &neg_out_pdfs,
	[NEG_OUT_MANY_PNGS]  = &neg_out_pngs,
	[NEG_OUT_TYPE_MAX]   = NULL,
};

enum neg_output_type neg_get_output_type(const char *name)
{
	enum neg_output_type type;

	for (type = 0; type < NEG_OUT_TYPE_MAX; type++) {
		struct neg_output *out;

		out = handlers[type];

		if (!out || !out->name || 
				strcasecmp(out->name, name))
			continue;

		break;
	}

	return type;
}

struct neg_output *neg_get_output(enum neg_output_type type)
{
	struct neg_output *out = NULL;
	if (type < NEG_OUT_TYPE_MAX)
		out = handlers[type];
	return out;
}
