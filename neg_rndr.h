#ifndef __NEG_OUT_H__
#define __NEG_OUT_H__

#include <stdint.h>
#include <stdbool.h>
#include <cairo.h>

struct neg_conf;

enum neg_output_type {
	NEG_OUT_SINGLE_PDF, // first one is the default
	NEG_OUT_MANY_PDFS,
	NEG_OUT_MANY_PNGS,
	NEG_OUT_TYPE_MAX,
};

typedef void* neg_output_ctx;

struct neg_output {
	const char *name;

	neg_output_ctx (*init)(struct neg_conf *conf);
	cairo_surface_t* (*slide_start)(neg_output_ctx);
	bool (*slide_end)(neg_output_ctx);
	bool (*exit)(neg_output_ctx);
};

extern enum neg_output_type neg_get_output_type(const char *name);
extern struct neg_output *neg_get_output(enum neg_output_type);


#endif // __NEG_OUT_H__
