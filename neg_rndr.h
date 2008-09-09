#ifndef __NEG_RNDR_H__
#define __NEG_RNDR_H__

#include <stdint.h>
#include <stdbool.h>
#include <cairo.h>

struct neg_conf;

enum neg_render_type {
	NEG_RNDR_SINGLE_PDF, // first one is the default
	NEG_RNDR_MANY_PDFS,
	NEG_RNDR_MANY_PNGS,
	NEG_RNDR_TYPE_MAX,
};

typedef void* neg_render_ctx;

struct neg_render {
	const char *name;

	neg_render_ctx (*init)(struct neg_conf *conf);
	cairo_t* (*slide_start)(neg_render_ctx);
	bool (*slide_end)(neg_render_ctx);
	bool (*exit)(neg_render_ctx);
};

extern enum neg_render_type neg_get_render_type(const char *name);
extern struct neg_render *neg_get_renderer(enum neg_render_type);


#endif // __NEG_RNDR_H__
