#ifndef __NEG_CONF_H__
#define __NEG_CONF_H__

#include <stdint.h>
#include <string.h>

enum neg_output_type {
	NEG_OUT_SINGLE_PDF, // first one is the default
	NEG_OUT_MANY_PDFS,
	NEG_OUT_MANY_PNGS,
	NEG_OUT_TYPE_MAX,
};

#define NEG_OUT_SIZE_DONT_CARE 0

struct neg_conf {
	struct {
		const char *name;
	} in;
	struct {
		const char *name;
		enum neg_output_type type;
		double width, height;
	} out;
};

static inline void neg_conf_init(struct neg_conf *conf)
{
	memset(conf, 0, sizeof *conf);
}



#endif // __NEG_CONF_H__
