#ifndef __NEG_RSVG_H__
#define __NEG_RSVG_H__

#include <rsvg.h>

enum neg_layer_flags {
	// extracted from svg
	NEG_LAYER_LONELY       = 0x0001,
	NEG_LAYER_HIDDEN       = 0x0002,
	NEG_LAYER_STICKY_ABOVE = 0x0004,
	NEG_LAYER_STICKY_BELOW = 0x0008,
	// state tracking
	NEG_LAYER_RESOLVING    = 0x4000,
	NEG_LAYER_RESOLVED     = 0x8000,
};
#define NEG_LAYER_STICKY (NEG_LAYER_STICKY_ABOVE | NEG_LAYER_STICKY_BELOW)

struct neg_order {
	unsigned size;
	unsigned count;
	unsigned array[0];
};

extern struct neg_order *neg_order_new(unsigned size);
extern void neg_order_free(struct neg_order *);
extern void neg_order_add(struct neg_order *, unsigned);
extern void neg_order_copy(struct neg_order *, const struct neg_order *);
extern void neg_order_append(struct neg_order *, const struct neg_order *);

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
		struct neg_order     *local; // before sticky layers
		struct neg_order     *order; // includes sticky layers
	} *layers;

	struct neg_order *sticky_below;
	struct neg_order *sticky_above;
};

extern struct neg_rsvg *neg_rsvg_open(const char *name);
extern void neg_rsvg_close(struct neg_rsvg *rsvg);

#endif
