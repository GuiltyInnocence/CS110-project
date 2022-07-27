#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <sys/time.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PI 3.14159

#define FULLRUN
#define DBG(i) printf("%d\n",(i));

#define BSIZE 100

typedef struct FVec {
  unsigned int length;
  unsigned int min_length;
  unsigned int min_deta; //max deta
  float *data;
  float *sum;
} FVec;

typedef struct Image {
  unsigned int dimX, dimY, numChannels;
  float *data;
} Image;

typedef struct TImg {
  unsigned int dX, dY, halflength;
  float *data;
} TImg;


inline float *get_pixel_no2(Image img, int x, int y) {
  return img.data + 3 * (y * img.dimX + x);
}

inline float *get_pixel_nox(Image img, int x, int y) {
  if (y < 0) y = 0;
  if (y >= img.dimY) y = img.dimY - 1;
  return img.data + 3 * (y * img.dimX + x);
}

inline float *get_pixel_noy(Image img, int x, int y) {
  if (x < 0) x = 0;
  if (x >= img.dimX) x = img.dimX - 1;
  return img.data + 3 * (y * img.dimX + x);
}

inline float gd(float a, float b, float x) {
  float c = ((x - b) / a);
  return exp((-.5) * c * c) / (a * sqrt(2 * PI));
}

FVec make_gv(float a, float x0, float x1, unsigned int length, unsigned int min_length) {
  FVec v;
  int ext = length >> 1;
  unsigned int i, j;
  v.length = length;
  v.min_length = min_length;
  if (v.min_length > v.length) 
    v.min_deta = 0;
  else 
    v.min_deta = ((v.length - v.min_length) >> 1);
  v.data = malloc(length * sizeof(float));
  v.sum = malloc((ext + 1) * sizeof(float));
  float step = (x1 - x0) / ((float)length);
  for (int i = 0; i < length; i++) 
    v.data[i] = gd(a, 0.0f, (i - ext) * step);
  // normalize_FVec(v);
  v.sum[0] = v.data[ext];
  for (i = ext + 1, j = 1; i < v.length; i++, j++) // loop length/2 times
    v.sum[j] = v.sum[j - 1] + v.data[i] * 2;
  return v;
}

TImg gb_h(TImg a, FVec gv){
  

}

Image gb_v(TImg a, FVec gv){
  

}

int main(int argc, char **argv) {
#ifdef FULLRUN
  struct timeval start_time, stop_time, elapsed_time;
  gettimeofday(&start_time, NULL);
  if (argc < 6) {
    printf("Usage: ./gb.exe <inputjpg> <outputname> <float: a> <float: x0> "
           "<float: x1> <unsigned int: dim>\n");
    exit(0);
  }
#endif

  float a, x0, x1;
  unsigned int dim, min_dim;

  sscanf(argv[3], "%f", &a);
  sscanf(argv[4], "%f", &x0);
  sscanf(argv[5], "%f", &x1);
  sscanf(argv[6], "%u", &dim);
  sscanf(argv[7], "%u", &min_dim);

  FVec v = make_gv(a, x0, x1, dim, min_dim);
  // print_fvec(v);
  Image img;
  img.data = stbi_loadf(argv[1], &(img.dimX), &(img.dimY), &(img.numChannels), 0);
/*  
  for(int i=0;i<img.dimX;i++){
    for(int j=0;j<img.dimY;j++)
      printf("%3.0f %3.0f %3.0f ",img.data[(i*img.dimX+j)*3],img.data[(i*img.dimX+j)*3+1],img.data[(i*img.dimX+j)*3+2]);
    puts("");
  }
*/
  TImg expand; // x expand
  expand.dX = img.dimX + dim - 1;
  expand.dY = img.dimY;
  expand.halflength = dim >> 1;
  expand.data = malloc(expand.dX * expand.dY * sizeof(float));
  unsigned int siz = expand.dX*expand.dY;
  for(int i=0;i<expand.dY; ++i) {
    int n1=i*expand.dX;
    int n2=siz+i*expand.dX;
    int n3=(siz>>1)+i*expand.dX;
    for(int j=0; j<dim>>1; ++j){
      expand.data[n1++] = img.data[i*img.dimX*3];
      expand.data[n2++] = img.data[i*img.dimX*3+1];
      expand.data[n3++] = img.data[i*img.dimX*3+2];
    }
    // memcpy(expand.data+i*expand.dX+(dim>>1), img.data+i*img.dimX, img.dimX*sizeof(float));
    n1=i*expand.dX;
    n2=siz+i*expand.dX;
    n3=(siz>>1)+i*expand.dX;
    for(int j=0; j<img.dimX; ++j){
      expand.data[n1++] = img.data[(i*img.dimX+j)*3];
      expand.data[n2++] = img.data[(i*img.dimX+j)*3+1];
      expand.data[n3++] = img.data[(i*img.dimX+j)*3+2];
    }
    n1=i*expand.dX+img.dimX+(dim>>1);
    n2=siz+i*expand.dX+img.dimX+(dim>>1);
    n3=(siz>>1)+i*expand.dX+img.dimX+(dim>>1);
    for(int j=0; j<dim>>1; ++j){
      expand.data[n1++] = img.data[(i*img.dimX+img.dimX-1)*3];
      expand.data[n2++] = img.data[(i*img.dimX+img.dimX-1)*3+1];
      expand.data[n3++] = img.data[(i*img.dimX+img.dimX-1)*3+2];
    }
  }
  TImg b = gb_h(expand, v);
  Image imgOut = gb_v(b, v);

  stbi_write_jpg(argv[2], imgOut.dimX, imgOut.dimY, imgOut.numChannels,
                 imgOut.data, 90);

#ifdef FULLRUN
  gettimeofday(&stop_time, NULL);
  timersub(&stop_time, &start_time, &elapsed_time);
  printf("%f \n", elapsed_time.tv_sec + elapsed_time.tv_usec / 1000000.0);
#endif
  free(expand.data);
  free(b.data);
  free(imgOut.data);
  free(v.data);
  free(v.sum);
  return 0;
}

