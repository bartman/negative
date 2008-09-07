#ifndef __NEG_LOAD_RSVG_H__
#define __NEG_LOAD_RSVG_H__

#include <rsvg.h>

struct neg_rsvg {
	RsvgHandle *handle;
	RsvgDimensionData size;

	unsigned    layer_count;
	struct neg_layer {
		const char *id;
		const char *label;
	} **layers;
};

extern struct neg_rsvg *neg_rsvg_open(const char *name);

extern void neg_rsvg_close(struct neg_rsvg *rsvg);

#endif
