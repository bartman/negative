#include <err.h>
#include <string.h>
#include <strings.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <rsvg.h>
#include <rsvg-cairo.h>
#include <stdlib.h>

#define OUTPUT_X 1024
#define OUTPUT_Y 768

enum output_format{
	OUT_PNG,
	OUT_PDF,
	OUT_MAX,
};

static struct fmt_name {
	const char *name;
} fmt_name_table[OUT_MAX+1] = {
	[OUT_PNG] = { .name = "png" },
	[OUT_PDF] = { .name = "pdf" },
	[OUT_MAX] = { .name = NULL },
};

static enum output_format fmt_name_to_enum(const char *name)
{
	enum output_format i;

	for (i=0; i<OUT_MAX; i++) {
		if (!strcasecmp(fmt_name_table[i].name, name))
			break;
	}
	return i;
}

static const char *fmt_enum_name(enum output_format fmt)
{
	if (fmt < OUT_MAX)
		return fmt_name_table[fmt].name;
	return NULL;
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
	unsigned int argi, slide = 0;
	RsvgHandle *rsvg;
	GError *err;
	double output_x = OUTPUT_X;
	double output_y = OUTPUT_Y;
	RsvgDimensionData rsvg_size;
	enum output_format fmt = OUT_PNG;

	for (argi = 1; argi < argc; argi++) {
		int cmd;

		// only support short arguments
		if (argv[argi][0] != '-' || argv[argi][2])
			break;

		cmd = argv[argi][1];

		switch (cmd) {
		case 'h':
			printf("%s [-h] [-x <width>] [-y <height>] [-o png|pdf]\n",
					argv[0]);
			return 0;
		case 'x':
			argi++;
			output_x = atoi(argv[argi]);
			if (output_x < 512 || output_x > 10000)
				errx(1, "-x '%s' is invalid", argv[argi]);
			break;
		case 'y':
			argi++;
			output_y = atoi(argv[argi]);
			if (output_y < 512 || output_y > 10000)
				errx(1, "-y '%s' is invalid", argv[argi]);
			break;
		case 'o':
			argi++;
			fmt = fmt_name_to_enum(argv[argi]);
			if ((unsigned)fmt > OUT_MAX)
				errx(1, "-o '%s' is invalid", argv[argi]);
			break;
		default:
			errx(1, "Unhandled option %s", argv[argi-1]);
			break;
		}
	}

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(argv[argi], &err);
	if (!rsvg)
		errx(1, "Could not load file %s", argv[argi]);
	argi++;

	rsvg_handle_get_dimensions(rsvg, &rsvg_size);

	printf("format %s\n", fmt_enum_name(fmt));
	printf("input %u x %u\n", rsvg_size.width, rsvg_size.height);
	printf("output %u x %u\n", (unsigned)output_x, (unsigned)output_y);

	for (; argi < argc; argi++) {
		cairo_surface_t* csurf;
		cairo_t *c;
		const char *p;
		char filename[sizeof("slide-%03i.xxx")];

		sprintf(filename, "slide-%03i.%s", ++slide, fmt_enum_name(fmt));
		printf("%s\n", filename);

		switch (fmt) {
		case OUT_PNG:
			csurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					output_x, output_y);
			break;

		case OUT_PDF:
			csurf = cairo_pdf_surface_create(filename,
					output_x, output_y);
			break;

		default:
			errx(1, "Unexpected format index %u", fmt);
		}
		if (!csurf)
			errx(1, "Could not create %s surface",
					fmt_enum_name(fmt));
		c = cairo_create(csurf);
		if (!c)
			errx(1, "Could not create cairo");

		cairo_scale(c, output_x/rsvg_size.width,
				output_y/rsvg_size.height);

		for (p = argv[argi]; p; p = next_layer(p)) {
			char id[1 + strcspn(p, ",") + 1];
			id[0] = '#';
			memcpy(id+1, p, strcspn(p, ","));
			id[1+strcspn(p, ",")] = '\0';

			printf(" - %s\n", id);
			rsvg_handle_render_cairo_sub(rsvg, c, id);
		}

		switch (fmt) {
		case OUT_PNG:
			if (cairo_surface_write_to_png(csurf, filename)
					!= CAIRO_STATUS_SUCCESS)
				errx(1, "writing out %s", filename);
			break;

		case OUT_PDF:
			cairo_surface_flush(csurf);
			break;

		default:
			errx(1, "Unexpected format index %u", fmt);
		}
		cairo_destroy(c);
		cairo_surface_destroy(csurf);
	}
	return 0;
}
