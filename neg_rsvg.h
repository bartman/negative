#ifndef __NEG_RSVG_H__
#define __NEG_RSVG_H__

#include <rsvg.h>

enum neg_layer_flags {
	NEG_LAYER_HIDDEN       = 0x0001,
	NEG_LAYER_STICKY_ABOVE = 0x0002,
	NEG_LAYER_STICKY_BELOW = 0x0004,
};

struct neg_rsvg {
	RsvgHandle *handle;
	RsvgDimensionData size;

	unsigned    layer_count;        // how many we have
	unsigned    layer_size;         // how many we allocated for
	struct neg_layer {
		// read from rsvg file
		const char *id;
		const char *label;

		// inferred
		const char           *name;
		unsigned             name_len;
		enum neg_layer_flags flags;
		unsigned             *order;
		unsigned             order_count;
	} *layers;

};

extern struct neg_rsvg *neg_rsvg_open(const char *name);

extern void neg_rsvg_close(struct neg_rsvg *rsvg);

#endif
