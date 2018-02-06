//Brian Choromanski
//CS449 Project 1 Part 2
//Lecture Section 19730 (Tu/Th 11:00-12:15)
//Recitation Section 20024 (Tu 3:00-3:50)

#include <stdio.h>
#include <string.h>
#include <math.h>
#pragma pack(1)

struct bmpHeader{		//Struct for the Header
	unsigned char format_identifier[2];		//2 bytes for the FormatIdentifier
	unsigned int size_of_file;			//4 bytes for the size of file
	unsigned short reserved_value;			//2 bytes for the reserved value
	unsigned short another_reserved_value;		//2 bytes for the second reserved value
	unsigned int offset;				//4 bytes for the offset to start the pixel array
};

struct dibHeader{		//Struct for the DIB header
	unsigned int size_of_DIB;			//4 bytes for the size of the DIB header in bytes
	unsigned int width_in_pixels;			//4 bytes for the width of the image in pixels
	unsigned int height_in_pixels;			//4 bytes for the height of the image in pixels
	unsigned short number_of_color_planes;		//2 bytes for the number of color 
	unsigned short number_bits_per_pixel;		//2 bytes for the number of bits per pixel
	unsigned int compression_scheme;		//4 bytes for the compression scheme used
	unsigned int image_size;			//4 bytes for the image size in bytes
	unsigned int horizontal_resolution;		//4 bytes for the horizontal resolution
	unsigned int vertical_resolution;		//4 bytes for the vertical resolution
	unsigned int number_of_colors_in_palette;	//4 bytes for the number of colors in palette
	unsigned int number_of_important_colors;	//4 bytes for the number of important colors
};

struct pixel{			//Struct for the pixel
	unsigned char blue_intensity;			//1 bytes for the blue intensity
	unsigned char green_intensity;			//1 bytes for the green intensity
	unsigned char red_intensity;			//1 bytes for the red intensity
};

int main(int argc, char **argv){

	FILE *f = fopen(argv[2], "r+b");	//Opens the binary file for both reading and writing
	struct bmpHeader header;					
	struct dibHeader dib;
	struct pixel color;
	if(f == NULL){				//Checks if the file was opened or not
		printf("Error in opening file\n");
		return 1;
	}
	fread(&header, sizeof(header), 1, f);	//Reads the Header of the bmp file into the struct element "header"
	if(header.format_identifier[0] != 'B' || header.format_identifier[1] != 'M'){	//Makes sure the format identifier in the header is equal to "BM"
		printf("Format identifier is \"%c%c\" it should be \"BM\"", (char)header.format_identifier[0], (char)header.format_identifier[1]);
		return 1;
	}
	fread(&dib, sizeof(dib), 1, f);		//Reads the DIB header of the bmp file into the struct element "dib"
	if(dib.size_of_DIB != 40){		//Makes sure that the size of the DIB header is equal to 40 bytes
		printf("Size of dib is %i it should be 40", dib.size_of_DIB);
		return 1;
	}
	if(dib.number_bits_per_pixel != 24){	//Makes sure that the number of bits per pixel is equal to 24 bits
		printf("Bits per pixel not 24");
		return 1;
	}
	fseek(f, header.offset, SEEK_SET);	//Sets the file position of the stream to the offset to start the pixel array
	int padding = (4-((sizeof(color) * dib.width_in_pixels)%4))%4;		//Calculates the size of empty padding at the end of every row of pixels 
	if(strcmp(argv[1], "-invert") == 0){	//Checks to see if the first argument is "-invert"
		int i, j;				//Creates i and j counter variables
		for(i = 0; i < dib.height_in_pixels; i++){	//Loops through the rows
			for(j = 0; j < dib.width_in_pixels; j++){	//Loops through each pixel in every row
				fread(&color, sizeof(color), 1, f);			//Reads the current pixel into the struct element "color"
				color.blue_intensity = (char)~color.blue_intensity;	//Saves the bitwise-NOT of the blue intensity into the blue intensity field of the struct element "color"
				color.green_intensity = (char)~color.green_intensity;	//Saves the bitwise-NOT of the green intensity into the green intensity field of the struct element "color"
				color.red_intensity = (char)~color.red_intensity;	//Saves the bitwise-NOT of the red intensity into the red intensity field of the struct element "color"
				fseek(f, -sizeof(color), SEEK_CUR);			//Sets the file position of the stream to the beginning of the pixel we just read
				fwrite(&color, sizeof(color), 1, f);			//Writes the modified struct element "color" over the pixel we just read
			}	
			fseek(f, padding, SEEK_CUR);			//Increments the file position of the stream by the amount of empty padding
		}
		
	}else if(strcmp(argv[1], "-grayscale") == 0){	//Checks to see if the first argument is "-greyscale"
		int i, j;				//Initialize i and j counter variables
		for(i = 0; i < dib.height_in_pixels; i++){	//Loops through the rows
			for(j = 0; j < dib.width_in_pixels; j++){	//Loops through each pixel in every row
				fread(&color, sizeof(color), 1, f);				//Reads the current pixel into the struct element "color"
				float blue = ((float)color.blue_intensity) / 255;		//Normalizes the blue intensity in the range [0,1]
				float green = ((float)color.green_intensity) / 255;		//Normalizes the green intensity in the range [0,1]
				float red = ((float)color.red_intensity) / 255.;		//Normalizes the red intensity in the range [0,1]
				float y = (0.2126 * red) + (0.7152 * green) + (0.0722 * blue);	//Combines blue, green, and red with a formula given to us to adjust how our eyes see it
				if(y > 0.0031308){						//If y > 0.0031308 then we use a function with a power to make it greyscale
					y = ((1.055*(pow(y,1/2.4)))-0.055);	
					
				}else{								//If y <= 0.0031308 then we use a different function to make it greyscale
					y = (12.92*y);
				}				
				color.blue_intensity = (char)(y*255);				//Saves the greyscale value multiplied by 255 to the blue intensity field of the struct element "color"
				color.green_intensity = (char)(y*255);			//Saves the greyscale value multiplied by 255 to the green intensity field of the struct element "color"
				color.red_intensity = (char)(y*255);				//Saves the greyscale value multiplied by 255 to the red intensity field of the struct element "color"
				fseek(f, -sizeof(color), SEEK_CUR);				//Sets the file position of the stream to the beginning of the pixel we just read
				fwrite(&color, sizeof(color), 1, f);				//Writes the modified struct element "color" over the pixel we just read
			}
			fseek(f, padding, SEEK_CUR);			//Increments the file position of the stream by the amount of empty padding
		}
	}
	fclose(f);		//Closes the binary file
	return 1;
}

