/**
* Dynamic and Iterative Spanning Forest (C)
*
* @date September, 2019
*/

//=============================================================================
// Includes
//=============================================================================
#include "Image.h"
#include "DISF.h"
#include "Utils.h"

#include <time.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//=============================================================================
// Prototypes
//=============================================================================
void usage();
Image *loadImage(const char* filepath);
void writeImagePGM(Image *img, char* filepath);

//=============================================================================
// Main
//=============================================================================
int main(int argc, char* argv[])
{
    int n_0, n_f;
    Image *img, *border_img, *label_img;
    Graph *graph;

    // if(argc != 4) usage();
    // img = loadImage(argv[1]);
    // n_0 = atoi(argv[2]);
    // n_f = atoi(argv[3]);

    // if(n_0 <= 1) printError("main", "N0 must be > 1");
    // else if(n_f <= 1) printError("main", "Nf must be > 1");
    // else if(n_0 < n_f) printError("main", "N0 must be >> Nf");
    
    img = loadImage("man.png");
    n_0 = 8000;
    n_f = 50;

    border_img = createImage(img->num_rows, img->num_cols, 1);
    graph = createGraph(img);
    freeImage(&img);

    label_img = runDISF(graph, n_0, n_f, &border_img);
    freeGraph(&graph);

    writeImagePGM(label_img, "labels.pgm");
    writeImagePGM(border_img, "borders.pgm");

    freeImage(&label_img);
    freeImage(&border_img);
}

//=============================================================================
// Methods
//=============================================================================
void usage()
{
    printf("Usage: DISF_demo <1> <2> <3>\n");
    printf("----------------------------------\n");
    printf("INPUTS:\n");
    printf("<1> - Image (STB's supported formats)\n" );
    printf("<2> - Initial number of seeds (e.g., N0 = 8000)\n");
    printf("<3> - Final number of superpixels (e.g., Nf = 50)\n");
    printError("main", "Too many/few parameters");
}

Image *loadImage(const char* filepath)
{
    int num_channels, num_rows, num_cols;
    unsigned char *data;    
    Image *new_img;
    
    data = stbi_load(filepath, &num_cols, &num_rows, &num_channels, 0);

    if(data == NULL)
        printError("loadImage", "Could not load the image <%s>", filepath);

    new_img = createImage(num_rows, num_cols, num_channels);

    #pragma omp parallel for
    for(int i = 0; i < new_img->num_pixels; i++)
    {
        new_img->val[i] = (int*)calloc(new_img->num_channels, sizeof(int));

        for(int j = 0; j < new_img->num_channels; j++)
        {
            new_img->val[i][j] = data[i * new_img->num_channels + j];
        }
    }

    stbi_image_free(data);

    return new_img;
}

void writeImagePGM(Image *img, char* filepath)
{
    int max_val, min_val;
    FILE *fp;

    fp = fopen(filepath, "wb");

    if(fp == NULL)
        printError("writeImagePGM", "Could not open the file <%s>", filepath);

    max_val = getMaximumValue(img, -1);
    min_val = getMinimumValue(img, -1);

    fprintf(fp, "P5\n");
    fprintf(fp, "%d %d\n", img->num_cols, img->num_rows);
    fprintf(fp, "%d\n", max_val);

    // 8-bit PGM file
    if(max_val < 256 && min_val >= 0)
    {
        unsigned char* data;

        data = (unsigned char*)calloc(img->num_pixels, sizeof(unsigned char));

        for(int i = 0; i < img->num_pixels; i++)
            data[i] = (unsigned char)img->val[i][0];

        fwrite(data, sizeof(unsigned char), img->num_pixels, fp);

        free(data);
    }
    // 16-bit PGM file
    else if(max_val < 65536 && min_val >= 0) 
    {
        unsigned short* data;

        data = (unsigned short*)calloc(img->num_pixels, sizeof(unsigned short));

        for(int i = 0; i < img->num_pixels; i++)
            data[i] = (unsigned short)img->val[i][0];
        
        for(int i = 0; i < img->num_pixels; i++)
        {
            int high, low;

            high = ((data[i]) & 0x0000FF00) >> 8;
            low = (data[i]) & 0x000000FF;

            fputc(high,fp);
            fputc(low, fp);
        }

        free(data);   
    }
    else
        printError("writeImagePGM", "Invalid min/max spel values <%d,%d>", min_val, max_val);

    fclose(fp);
}