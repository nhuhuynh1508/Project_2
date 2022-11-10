#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// include the headers
#define STB_IMAGE_IMPLEMENTATION
#include "./headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./headers/stb_image_write.h"

// convert the images into grayscale
void greyConverter(unsigned char *image_grey, unsigned char *image, int width, int height, int channel)
{
    int sum;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            sum = 0;
            for(int k = 0; k < channel; k++)
            {
                int index = i*width*channel + j*channel + k;
                sum += image[index];
            }
            image_grey[i*width+j] = sum / 3;
        }
    }
}

// calculate cosine similarity
// find the max value of cosine similarity to find the domain that is similar to the template
int cosine_similarity(unsigned char *template, unsigned char *image, int width, int height, int width_tem, int height_tem)
{
    double dot, denom_a, denom_b, result, max; 
    max = 0;
    int tem_value, img_value, best_x, best_y; 
    for (int a = 0; a < width - width_tem; a++)
    {
        for (int b = 0; b < height - height_tem; b++)
        {
            dot = 0;
            denom_a = 0;
            denom_b = 0;
            for (int i = 0; i < width_tem; i++) 
            {
                for (int j = 0;  j < height_tem; j++)
                {
                        int index_1 = (b+j)*width + (a+i);
                        int index_2 = j*width_tem + i;
                        img_value = image[index_1];
                        tem_value = template[index_2];
                        
                        dot += tem_value*img_value;
                        denom_a += tem_value*tem_value;
                        denom_b += img_value*img_value;
                }
            }
            result = dot / (sqrt(denom_a) * sqrt(denom_b));
           
            if (result > max)
            {
                max = result;
                best_x = a;
                best_y = b;
            }
        }
    }
    return best_x + best_y*width;
}

// draw a rectangle around the domain
void drawRect(unsigned char *image, int index, int width, int height, int channel, int width_tem, int height_tem)
{
    int best_x, best_y;
    best_y = index / width;
    best_x = index % width;
    int index_drawing;
    for (int a = 0; a < width_tem; a++)
    {
        for (int b = 0; b < height_tem; b++)
        {
            if ((a == 0) || (b == 0) || (a == width_tem-1) || (b == height_tem-1))
            {
                index_drawing = (b+best_y)*width*channel + (a+best_x)*channel;
                for (int c = 0; c < channel; c++)
                {
                    image[index_drawing+c] = 0;
                }
            }
        }
    }
}

// update the template
void updateTemplate(unsigned char *image, unsigned char *template, int index, int width, int height, int channel, int width_tem, int height_tem)
{
    int best_x, best_y;
    best_y = index / width;
    best_x = index % width;
    int index_template;
    for (int a = 0; a < width_tem; a++)
    {
        for (int b = 0; b < height_tem; b++)
        {
            for (int k = 0; k < 3; k++)
            {
                index = (best_y+b)*width*channel + (best_x+a)*channel+k;
                index_template = b*width_tem*channel + a*channel+k;
                template[index_template] = image[index];
            }
        }
    }
}

int main()
{
    // declare variables
    int width, height, channel, width_tem, height_tem, channel_tem;
    int index;
    int image_num = 63;
    char path_template[] = "./data/template.jpg";
    char path_image[50];
    char path_save[50];
    unsigned char *object;
    unsigned char *image, *template;
    unsigned char *image_grey;
    unsigned char *template_grey;

    clock_t begin = clock();
    
    template = stbi_load(path_template, &width_tem, &height_tem, &channel_tem, 0);

    // loop through the images
    for (int count = 0; count < image_num; count++)
    {
        clock_t tic = clock();
        // store output
        sprintf(path_image, "./data/images/img%d.jpg", count);
        sprintf(path_save, "./results/result_img%d.jpg", count);
       
        image = stbi_load(path_image, &width, &height, &channel, 0);
    
        if (template == NULL || image == NULL)
        {
            printf("\nError in loading the image\n");
            exit(1);
        }

        printf ("W = %d\nH = %d\nC = %d\n" ,width, height, channel);
        printf ("W_temp = %d\nH_temp = %d\nC_temp = %d\n" ,width_tem, height_tem, channel_tem);

        image_grey = (unsigned char*)malloc(width*height*sizeof(unsigned char));
        template_grey = (unsigned char*)malloc(width_tem*height_tem*sizeof(unsigned char));
        
        greyConverter(image_grey, image, width, height, channel);
        greyConverter(template_grey, template, width_tem, height_tem, channel_tem);

        int index_best = cosine_similarity(template_grey, image_grey, width, height, width_tem, height_tem);
        printf("Y: %d\n", index_best/width);
        printf("X: %d\n", index_best%width);
        drawRect(image, index_best, width, height, channel, width_tem, height_tem);
        updateTemplate(image, template, index_best, width, height, channel, width_tem, height_tem);

        stbi_write_jpg(path_save, width, height, channel, image, width*channel);

        printf("Image saved to %s\n", path_save);

        clock_t toc = clock();
        double time = (double)(toc - tic) / CLOCKS_PER_SEC;
        printf("Processing time %f - image: %d\n", time, count);
    }
   
    free(image);
    free(image_grey);
    free(template);
    free(template_grey);
     
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Processing time: %f", time_spent);
}