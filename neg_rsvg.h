#ifndef __NEG_RSVG_H__
#define __NEG_RSVG_H__

#include <rsvg.h>

struct neg_rsvg {
	RsvgHandle *handle;
	RsvgDimensionData size;

	unsigned    layer_count;        // how many we have
	unsigned    layer_size;         // how many we allocated for
	struct neg_layer {
		const char *id;
		const char *label;
	} *layers;
};

extern struct neg_rsvg *neg_rsvg_open(const char *name);

extern void neg_rsvg_close(struct neg_rsvg *rsvg);

#endif
