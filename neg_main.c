#include <err.h>
#include <string.h>
#include <strings.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <rsvg.h>
#include <rsvg-cairo.h>
#include <stdlib.h>

#include "neg_conf.h"
#include "neg_opts.h"
#include "neg_state.h"
#include "neg_out.h"

static struct fmt_name {
	const char *name;
} fmt_name_table[NEG_OUT_TYPE_MAX+1] = {
	[NEG_OUT_SINGLE_PDF] = { .name = "1pdf" },
	[NEG_OUT_MANY_PDFS] = { .name = "pdf" },
	[NEG_OUT_MANY_PNGS] = { .name = "png" },
	[NEG_OUT_TYPE_MAX] = { .name = NULL },
};

enum neg_output_type fmt_name_to_enum(const char *name)
{
	enum neg_output_type i;

	for (i=0; i<NEG_OUT_TYPE_MAX; i++) {
		if (!strcasecmp(fmt_name_table[i].name, name))
			break;
	}
	return i;
}

static const char *next_layer(const char *p)
{
	const char *comma = strchr(p, ',');
	if (!comma)
		return NULL;
	return comma+1;
}

int main(int argc, char *argv[])
{
	int argi;
	unsigned int slide = 0;
	RsvgHandle *rsvg;
	GError *err;
	RsvgDimensionData rsvg_size;
	struct neg_conf conf;
	struct neg_output *out;

	neg_program = argv[0];

	argi = neg_parse_cmdline(&conf, argc, argv);
	conf.out.type = NEG_OUT_MANY_PNGS;

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(conf.in.name, &err);
	if (!rsvg)
		errx(1, "Could not load file %s", conf.in.name);

	rsvg_handle_get_dimensions(rsvg, &rsvg_size);

	// {{{ TODO shoudl go into neg_set_defaults() or some such
	if (!conf.out.width)
		conf.out.width = rsvg_size.width;
	if (!conf.out.height)
		conf.out.height = rsvg_size.height;
	if (!conf.out.name)
		conf.out.name = "slide-%03u.%s";
	// }}}

	out = neg_get_output(conf.out.type);
	if (!out)
		errx(1, "Could not handler output type #%u", conf.out.type);

	printf("format %s\n", out->name);
	printf("input  %u x %u\n", rsvg_size.width, rsvg_size.height);
	printf("output %u x %u\n", (unsigned)conf.out.width, (unsigned)conf.out.height);

	for (; argi < argc; argi++) {
		cairo_surface_t* csurf;
		cairo_t *c;
		const char *p;
		char filename[sizeof("slide-%03i.xxx")];

		sprintf(filename, "slide-%03i.%s", ++slide, out->ext);
		printf("%s\n", filename);

		switch (conf.out.type) {
		case NEG_OUT_MANY_PNGS:
			csurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					conf.out.width, conf.out.height);
			break;

		case NEG_OUT_MANY_PDFS:
			csurf = cairo_pdf_surface_create(filename,
					conf.out.width, conf.out.height);
			break;

		default:
			errx(1, "Unexpected format index %u", conf.out.type);
		}
		if (!csurf)
			errx(1, "Could not create %s surface", out->ext);
		c = cairo_create(csurf);
		if (!c)
			errx(1, "Could not create cairo");

		cairo_scale(c, conf.out.width/rsvg_size.width,
				conf.out.height/rsvg_size.height);

		for (p = argv[argi]; p; p = next_layer(p)) {
			char id[1 + strcspn(p, ",") + 1];
			id[0] = '#';
			memcpy(id+1, p, strcspn(p, ","));
			id[1+strcspn(p, ",")] = '\0';

			printf(" - %s\n", id);
			rsvg_handle_render_cairo_sub(rsvg, c, id);
		}

		switch (conf.out.type) {
		case NEG_OUT_MANY_PNGS:
			if (cairo_surface_write_to_png(csurf, filename)
					!= CAIRO_STATUS_SUCCESS)
				errx(1, "writing out %s", filename);
			break;

		case NEG_OUT_MANY_PDFS:
			cairo_surface_flush(csurf);
			break;

		default:
			errx(1, "Unexpected format index %u", conf.out.type);
		}
		cairo_destroy(c);
		cairo_surface_destroy(csurf);
	}
	return 0;
}
