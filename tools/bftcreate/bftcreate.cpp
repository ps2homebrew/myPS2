// bftcreate.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	FILE			*fp;
	unsigned char	ident_field_size;
	unsigned char	color_map_type;
	unsigned char	image_type;
	unsigned short	x_origin, y_origin;
	unsigned short	width, height;
	unsigned char	bits_per_pixel;
	unsigned char	image_specs;
	int				i;

	int				num_pixels;
	unsigned int	*pixel_data;
	int				rows;

	unsigned char	*char_widths;

	if( argc < 4 ) {
		fprintf(stderr, "bftcreate.exe <font_tga> <width_dat> <output_bft>\n");
		return 0;
	}

	if( (fp = fopen(argv[1], "rb")) == NULL ) {
		fprintf(stderr, "Couldn't open tga file %s\n", argv[1]);
		return 1;
	}

	fread( &ident_field_size, 1, 1, fp );
	fread( &color_map_type, 1, 1, fp );
	fread( &image_type, 1, 1, fp );

	if( color_map_type != 0 || image_type != 2 ) {
		fprintf(stderr, "Unsupported image type (%i)\n", image_type);
		fprintf(stderr, "Only uncompressed RGBA images are supported (type 2).\n");
		return 1;
	}

	// skip color map
	fseek( fp, 5, SEEK_CUR );

	fread( &x_origin, 2, 1, fp );
	fread( &y_origin, 2, 1, fp );
	fread( &width, 2, 1, fp );
	fread( &height, 2, 1, fp );
	fread( &bits_per_pixel, 1, 1, fp );

	if( bits_per_pixel != 32 ){
		fprintf(stderr, "Only 32bit RGBA images supported.\n");
		return 1;
	}

	if( width != 128 && width != 256 && width != 512 && width != 1024) {
		fprintf(stderr, "Invalid image dimensions (supported 128,256,512,1024).\n");
		return 1;
	}

	if( width != height ) {
		fprintf(stderr, "Image dimensions must be identical.\n");
		return 1;
	}

	fread( &image_specs, 1, 1, fp );
	
	if( !((image_specs >> 5) & 1) ) {
		fprintf(stderr, "Invalid TGA file. Image Origin must be in upper left-hand corner.\n");
		return 1;
	}

	// skip identification field
	if( ident_field_size != 0 )
		fseek( fp, ident_field_size, SEEK_CUR );

	pixel_data = (unsigned int*) malloc( width * height * (bits_per_pixel >> 3) );
	if(!pixel_data){
		fprintf(stderr, "Couldn't allocate memory for pixel data.\n");
		return 1;
	}

	fprintf(stderr, "Reading TGA RGBA data...\n");
	num_pixels = 0;
fread( pixel_data, width * height * 4, 1, fp );/*
	// read image data
	while( num_pixels < width*height )
	{
		unsigned int rgba;

		// tga images have their origin at the lower left corner
		// so we need to store them backwards to get them in the right order
		line	= num_pixels / width;
		offset	= num_pixels % width;

		fread( &rgba, 4, 1, fp );
		pixel_data[ width*height - line*width - (width-offset) ] = rgba;
		num_pixels++;
	}
*/
	fclose(fp);

	if( (fp = fopen(argv[2], "rb")) == NULL ) {
		fprintf(stderr, "Couldn't open dat file %s\n", argv[2]);
		return 1;
	}

	// fixme: should be 256
	char_widths = (unsigned char*) malloc( 512 );
	if( !char_widths ) {
		fprintf(stderr, "Couldn't allocate memory for dat data.\n");
		return 1;
	}

	fread( char_widths, 512, 1, fp );
	fclose(fp);

	if( (fp = fopen(argv[3], "wb")) == NULL ) {
		fprintf(stderr, "Couldn't create bft file %s\n", argv[3]);
		return 1;
	}

	fprintf(stderr, "Writing bft file %s\n", argv[3]);

	// BFT0
	// Image Width
	fwrite( "BFT0", 4, 1, fp );
	fwrite( &width, 2, 1, fp );

	// only care for the first 6 rows of characters
	rows = (height/16) * 6;

	for( i = 0; i < width * rows; i++ ) {
		unsigned char a;

		// ps2 alpha blending ranges from 0x00 to 0x80
		a = (pixel_data[i] >> 24) & 0xFF;
		a /= 0x02;

		fwrite( &a, 1, 1, fp );
	}

	// write character widths for ascii chars 32 - 128
	int w, c = 0;

	for( i = 0; i < 96; i++ ) {

		w = char_widths[c];
		c += 2;

		// printf("Writing char width %i for char %c\n", w, i + 32 );
		fwrite( &w, 4, 1, fp );
	}

	fclose(fp);

	fprintf(stderr, "Done!\n");
	free(pixel_data);
	free(char_widths);
	return 0;
}
