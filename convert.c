#include <err.h>
#include <string.h>
#include <cairo.h>
#include <rsvg.h>
#include <rsvg-cairo.h>

#define OUTPUT_X 800
#define OUTPUT_Y 600


static const char *next_layer(const char *p)
{
	const char *comma = strchr(p, ',');
	if (!comma)
		return NULL;
	return comma+1;
}

int main(int argc, char *argv[])
{
	unsigned int i;
	RsvgHandle *rsvg;
	GError *err;

	rsvg_init();
	rsvg = rsvg_handle_new_from_file(argv[1], &err);
	if (!rsvg)
		errx(1, "Could not load file %s", argv[1]);

	for (i = 2; i < argc; i++) {
		cairo_surface_t* csurf;
		cairo_t *c;
		const char *p;
		char name[sizeof("slide-%03i.png")];

		sprintf(name, "slide-%03i.png", i-1);
		printf("%s\n", name);

		csurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
						   OUTPUT_X, OUTPUT_Y);
		if (!csurf)
			errx(1, "Could not create surface");
		c = cairo_create(csurf);
		cairo_scale(c, OUTPUT_X/1024.0, OUTPUT_Y/768.0);
		if (!c)
			errx(1, "Could not create cairo");

		for (p = argv[i]; p; p = next_layer(p)) {
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
