#include <err.h>
#include <string.h>
#include <strings.h>

#include "neg_load_rsvg.h"

RsvgHandle *neg_load_rsvg(const char *name)
{
	GError *err;
	RsvgHandle *rsvg;

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(name, &err);
	if (!rsvg)
		errx(1, "Could not load file %s", name);

	return rsvg;
}
