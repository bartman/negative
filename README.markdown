Negative - slideware software
Copyright (c) 2008, Bart Trojanowski <bart@jukie.net>

# About

This program generates a presentation from an inkscape svg file
containing specially named layers (see "Layer naming" below).  The
output can be either a collection of png files, a collection of pdf
files, or a single pdf that contains all the slides.

## License

The code is licensed as GPL v2 or, at your option, any later version.
Please see [http://www.gnu.org/licenses/gpl-2.0.txt].

## History

The initial prototype and idea for negative came from a blog post by
Rusty Russell: [http://ozlabs.org/~rusty/index.cgi/2006]

## Source code

The project is tracked in git, you can obtain it by cloning it:

        git clone git://github.com/bartman/negative.git

or

        git clone git://git.jukie.net/negative.git/

## Dependencies

To build you will need at least librsvg and libcairo.

## Building

After obtaining the sources just run make in the project directory.

        make

Optionally you can install with:

        make install


# Using

Edit your slideshow in inkscape and save it as an 'Inkscape SVG' which
will preserve some of the information that negative works with.  Then
run:

        negative yourfile.svg

By default negative generates a `yourfile.pdf` from the input file.

## Demo

Included in the project is a `demo.svg` which shows some of the
features.  In a clean clone you could run:

        make
        negative -t pdf demo.svg
        xpdf slides.pdf

or to generate png files:

        negative -t pngs demo.svg
        gqview *.png

# Layer naming

Layer names are composed of mostly alpha numeric strings and spaces.
Additionally a few characters that depict special handling of a given
layer.

Layers generate slides from the top layer, being the first page in the
output, to the bottom layer, being the last page in the output.  Unless
a slide name begins with a hash symbol (`#`) each layer will generate a
slide page in the output.

Special meaning for non alpha-numeric characters at start of layer name:

 - `#` this layer does not generate a slide (it's hidden)
 - `_` this layer will be rendered below any layer that generates a slide
 - `^` this layer will be rendered above any layer that generates a slide

The `#` character can be combined with either `_` or `^` to give the
presentation a common theme across all slides.  For example, adding
`#_border` could be used to place a rectangular border around the slide
which would be placed under all other slides.  Similarly a `#^sun` layer
could draw a translucent sun that is rendered over the right corner of
some slides.

Special meaning for non alpha-numeric characters found in the middle of
the layer name:

 - `_` this layer will be rendered below anything that matches the
   substring on the left of this character in the layer name.
 - `^` this layer will be rendered above anything that matches the
   substring on the right of this character in the layer name.


vim: set ts=8 et sw=8 tw=72 ft=mkd
