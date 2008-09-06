#ifndef __NEG_CONF_H__
#define __NEG_CONF_H__

#include <stdint.h>
#include <string.h>
#include "neg_rndr.h"

#define NEG_OUT_SIZE_DONT_CARE 0

struct neg_conf {
	struct {
		const char *name;
	} in;
	struct {
		const char *name;
		enum neg_render_type type;
		double width, height;
	} out;
};

static inline void neg_conf_init(struct neg_conf *conf)
{
	memset(conf, 0, sizeof *conf);

	conf->out.type = NEG_RNDR_MANY_PNGS; // TODO: for now this is the default
}



#endif // __NEG_CONF_H__
