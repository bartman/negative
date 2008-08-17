#include <err.h>
#include <string.h>
#include <cairo.h>
#include <rsvg.h>
#include <rsvg-cairo.h>
#include <stdlib.h>

#define OUTPUT_X 1024
#define OUTPUT_Y 768

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

	for (argi = 1; argi < argc; argi++) {
		int cmd;

		// only support short arguments
		if (argv[argi][0] != '-' || argv[argi][2])
			break;

		cmd = argv[argi][1];

		switch (cmd) {
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

	printf("input %u x %u\n", rsvg_size.width, rsvg_size.height);
	printf("output %u x %u\n", (unsigned)output_x, (unsigned)output_y);

	for (; argi < argc; argi++) {
		cairo_surface_t* csurf;
		cairo_t *c;
		const char *p;
		char name[sizeof("slide-%03i.png")];

		sprintf(name, "slide-%03i.png", ++slide);
		printf("%s\n", name);

		csurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
						   output_x, output_y);
		if (!csurf)
			errx(1, "Could not create surface");
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
		if (cairo_surface_write_to_png(csurf, name)
		    != CAIRO_STATUS_SUCCESS)
			errx(1, "writing out %s", name);
		cairo_destroy(c);
		cairo_surface_destroy(csurf);
	}
	return 0;
}
