#include <math.h>
#include <stdio.h>
#include <stdlib.h>
// #include <algorithm>
#include <immintrin.h>
#include <sys/time.h>
#include <time.h>
// inplement dymanic

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PI 3.14159

// extra overhead for timing
#define FULLRUN
#define DBG(i) printf("%d\n",(i));

#define BSIZE 100

typedef struct FVec {
  unsigned int length;
  unsigned int min_length;
  unsigned int min_deta; //max deta
  float *data;
  float *sum;
  float *rsum;
} FVec;

typedef struct Image {
  unsigned int dimX, dimY, numChannels;
  float *data;
} Image;

/*
void normalize_FVec(FVec v) {
  // float sum = 0.0;
  unsigned int i, j;
  int ext = v.length / 2;
  v.sum[0] = v.data[ext];
  for (i = ext + 1, j = 1; i < v.length; i++, j++) {
    v.sum[j] = v.sum[j - 1] + v.data[i] * 2;
  }
  // for (i = 0; i <= ext; i++)
  // {
  //      v.data[i] /= v.sum[v.length - ext - 1 ] ;
  //      printf("%lf ",v.sum[i]);
  // }
}
*/
/*
inline float *get_pixel(Image img, int x, int y) {
  if (x < 0) x = 0;
  if (x >= img.dimX) x = img.dimX - 1;
  if (y < 0) y = 0;
  if (y >= img.dimY) y = img.dimY - 1;
  return img.data + 3 * (y * img.dimX + x);
}
*/
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
  v.data = malloc((((length>>3)+1)<<3) * sizeof(float));
  v.sum = malloc((ext + 1) * sizeof(float));
  v.rsum = malloc((ext + 1) * sizeof(float));
  float step = (x1 - x0) / ((float)length);
  for (int i = 0; i < length; i++) 
    v.data[i] = gd(a, 0.0f, (i - ext) * step);
  // normalize_FVec(v);
  v.sum[0] = v.data[ext]; // the peak value
  v.rsum[0] = 1.0 / v.data[ext];
  for (i = ext + 1, j = 1; i < v.length; i++, j++){ // symmetric
    v.sum[j] = v.sum[j - 1] + v.data[i] * 2;
    v.rsum[j] = 1.0 / v.sum[j];
  }
  return v;
}


// cid263 Always with three channels

// /* original
Image gb_h(Image a, FVec gv) {
  Image b = (Image){a.dimX, a.dimY, a.numChannels, malloc(a.dimX * a.dimY * a.numChannels * sizeof(float))};
  int ext = gv.length >> 1;
  unsigned int x, y;
  float *pc, *tmp;
  float sum,sum2,sum3;
#pragma omp parallel for private(x, pc, sum, sum2, sum3, tmp)
    for (y = 0; y < a.dimY; y++) {
      pc = get_pixel_no2(b, 0, y);
      for (x = 0; x < a.dimX; x++) {
        unsigned int deta = fmin(fmin(a.dimY - y - 1, y), fmin(a.dimX - x - 1, x));
        deta = fmin(deta, gv.min_deta);
        sum = 0;
        sum2 = 0;
        sum3 = 0;
        for (int i = deta; i < gv.length - deta; i++) { // loop around 1000 times
          tmp = get_pixel_noy(a, x + i - ext, y);
          sum += gv.data[i]  * (*tmp); //
          sum2 += gv.data[i]  * (*(tmp+1)); //
          sum3 += gv.data[i]  * (*(tmp+2)); //
        }
        pc[0] = sum * gv.rsum[ext - deta];
        pc[1] = sum2 * gv.rsum[ext - deta];
        pc[2] = sum3 * gv.rsum[ext - deta];
        pc += 3;
      }
    }

  return b;
}

Image gb_v(Image a, FVec gv) {
  Image b = (Image){a.dimX, a.dimY, a.numChannels, malloc(a.dimX * a.dimY * a.numChannels * sizeof(float))};
  int ext = gv.length >> 1;
  unsigned int x, y;
  float *pc, *tmp;
  float sum,sum2,sum3;
#pragma omp parallel for private(y, pc, sum, sum2, sum3, tmp)
    for (x = 0; x < a.dimX; x++) {
      pc = get_pixel_no2(b, x, 0);
      for (y = 0; y < a.dimY; y++) {
        unsigned int deta = fmin(fmin(a.dimY - y - 1, y), fmin(a.dimX - x - 1, x));
        deta = fmin(deta, gv.min_deta);
        sum = 0;
        sum2 = 0;
        sum3 = 0;
        for (int i = deta; i < gv.length - deta; i++) {
          tmp = get_pixel_nox(a, x, y + i - ext);
          sum += gv.data[i]  * (*tmp); //
          sum2 += gv.data[i]  * (*(tmp+1)); //
          sum3 += gv.data[i]  * (*(tmp+2)); //
        }
        // 256=8*float(32)
        pc[0] = sum * gv.rsum[ext - deta];
        pc[1] = sum2 * gv.rsum[ext - deta];
        pc[2] = sum3 * gv.rsum[ext - deta];
        pc += 3 * a.dimX;
      }
    }
  return b;
}

// */

/* simd
Image gb_h(Image a, FVec gv) {
  // Image b = img_sc(a);
  Image b = (Image){a.dimX, a.dimY, a.numChannels, malloc(a.dimX * a.dimY * a.numChannels * sizeof(float))};

  int ext = gv.length >> 1;
  unsigned int x, y;
  float *pc, *tmp;
  float res[12], v_tmp[12], v_tmp2[12];
  __m128 v_sum[3];
#pragma omp parallel for private(x, y, pc,tmp, res, v_tmp, v_tmp2, v_sum)
    for (x = 0; x < a.dimX; x++) {
      pc = get_pixel(b, x, 0);
      for (y = 0; y < a.dimY; y++) {
        unsigned int deta = fmin(fmin(a.dimY - y - 1, y), fmin(a.dimX - x - 1, x));
        deta = fmin(deta, gv.min_deta);
        v_sum[0] = _mm_setzero_ps();
        v_sum[1] = _mm_setzero_ps();
        v_sum[2] = _mm_setzero_ps();
        for (int i = deta; i < (int)((gv.length - deta) / 4) * 4; i += 4) { // loop around 1000 times
          tmp = get_pixel(a, x + i - ext, y);
          v_tmp[0] = *(tmp++);
          v_tmp[1] = *(tmp++);
          v_tmp[2] = *tmp;
          tmp = get_pixel(a, x + i + 1 - ext, y);
          v_tmp[3] = *(tmp++);
          v_tmp[4] = *(tmp++);
          v_tmp[5] = *tmp;
          tmp = get_pixel(a, x + i + 2 - ext, y);
          v_tmp[6] = *(tmp++);
          v_tmp[7] = *(tmp++);
          v_tmp[8] = *tmp;
          tmp = get_pixel(a, x + i + 3 - ext, y);
          v_tmp[9] = *(tmp++);
          v_tmp[10] = *(tmp++);
          v_tmp[11] = *tmp;

          v_tmp2[0] = gv.data[i] / gv.sum[ext - deta];
          v_tmp2[1] = v_tmp2[0];
          v_tmp2[2] = v_tmp2[0];
          v_tmp2[3] = gv.data[i + 1] / gv.sum[ext - deta];
          v_tmp2[4] = v_tmp2[3];
          v_tmp2[5] = v_tmp2[3];
          v_tmp2[6] = gv.data[i + 2] / gv.sum[ext - deta];
          v_tmp2[7] = v_tmp2[6];
          v_tmp2[8] = v_tmp2[6];
          v_tmp2[9] = gv.data[i + 3] / gv.sum[ext - deta];
          v_tmp2[10] = v_tmp2[9];
          v_tmp2[11] = v_tmp2[9];

          v_sum[0] = _mm_add_ps(v_sum[0], _mm_mul_ps(_mm_loadu_ps(v_tmp), _mm_loadu_ps(v_tmp2)));
          v_sum[1] = _mm_add_ps(v_sum[1], _mm_mul_ps(_mm_loadu_ps(v_tmp+4), _mm_loadu_ps(v_tmp2+4)));
          v_sum[2] = _mm_add_ps(v_sum[2], _mm_mul_ps(_mm_loadu_ps(v_tmp+8), _mm_loadu_ps(v_tmp2+8)));
        }
        _mm_storeu_ps(res, v_sum[0]);
        _mm_storeu_ps(res + 4, v_sum[1]);
        _mm_storeu_ps(res + 8, v_sum[2]);
        pc[0] = res[0] + res[3] + res[6] + res[9];
        pc[1] = res[1] + res[4] + res[7] + res[10];
        pc[2] = res[2] + res[5] + res[8] + res[11];

        for (int i = (int)((gv.length - deta) / 4) * 4; i < gv.length - deta; i++){ // tail case
          tmp = get_pixel(a, x, y + i - ext);
          pc[0] += gv.data[i] / gv.sum[ext - deta] * (*tmp);
          pc[1] += gv.data[i] / gv.sum[ext - deta] * (*(tmp+1)); 
          pc[2] += gv.data[i] / gv.sum[ext - deta] * (*(tmp+2)); 
        }

        pc += 3 * a.dimX;
      }
    }
  return b;
}
Image gb_v(Image a, FVec gv) {
  // Image b = img_sc(a);
  Image b = (Image){a.dimX, a.dimY, a.numChannels, malloc(a.dimX * a.dimY * a.numChannels * sizeof(float))};

  int ext = gv.length >> 1;
  unsigned int x, y;
  float *pc, *tmp;
  float res[12], v_tmp[12], v_tmp2[12];
  __m128 v_sum[3];
#pragma omp parallel for private(x, y, pc, tmp, res, v_tmp, v_tmp2, v_sum)
    for (x = 0; x < a.dimX; x++) {
      pc = get_pixel(b, x, 0);
      for (y = 0; y < a.dimY; y++) {
        unsigned int deta = fmin(fmin(a.dimY - y - 1, y), fmin(a.dimX - x - 1, x));
        deta = fmin(deta, gv.min_deta);
        v_sum[0] = _mm_setzero_ps();
        v_sum[1] = _mm_setzero_ps();
        v_sum[2] = _mm_setzero_ps();
        for (int i = deta; i < (int)((gv.length - deta) / 4) * 4; i += 4) { // loop around 1000 times
          tmp = get_pixel(a, x, y + i - ext);
          v_tmp[0] = *(tmp++);
          v_tmp[1] = *(tmp++);
          v_tmp[2] = *tmp;
          tmp = get_pixel(a, x, y + i + 1 - ext);
          v_tmp[3] = *(tmp++);
          v_tmp[4] = *(tmp++);
          v_tmp[5] = *tmp;
          tmp = get_pixel(a, x, y + i + 2 - ext);
          v_tmp[6] = *(tmp++);
          v_tmp[7] = *(tmp++);
          v_tmp[8] = *tmp;
          tmp = get_pixel(a, x, y + i + 3 - ext);
          v_tmp[9] = *(tmp++);
          v_tmp[10] = *(tmp++);
          v_tmp[11] = *tmp;

          v_tmp2[0] = gv.data[i] / gv.sum[ext - deta];
          v_tmp2[1] = v_tmp2[0];
          v_tmp2[2] = v_tmp2[0];
          v_tmp2[3] = gv.data[i + 1] / gv.sum[ext - deta];
          v_tmp2[4] = v_tmp2[3];
          v_tmp2[5] = v_tmp2[3];
          v_tmp2[6] = gv.data[i + 2] / gv.sum[ext - deta];
          v_tmp2[7] = v_tmp2[6];
          v_tmp2[8] = v_tmp2[6];
          v_tmp2[9] = gv.data[i + 3] / gv.sum[ext - deta];
          v_tmp2[10] = v_tmp2[9];
          v_tmp2[11] = v_tmp2[9];

          v_sum[0] = _mm_add_ps(v_sum[0], _mm_mul_ps(_mm_loadu_ps(v_tmp), _mm_loadu_ps(v_tmp2)));
          v_sum[1] = _mm_add_ps(v_sum[1], _mm_mul_ps(_mm_loadu_ps(v_tmp+4), _mm_loadu_ps(v_tmp2+4)));
          v_sum[2] = _mm_add_ps(v_sum[2], _mm_mul_ps(_mm_loadu_ps(v_tmp+8), _mm_loadu_ps(v_tmp2+8)));
        }
        _mm_storeu_ps(res, v_sum[0]);
        _mm_storeu_ps(res + 4, v_sum[1]);
        _mm_storeu_ps(res + 8, v_sum[2]);
        pc[0] = res[0] + res[3] + res[6] + res[9];
        pc[1] = res[1] + res[4] + res[7] + res[10];
        pc[2] = res[2] + res[5] + res[8] + res[11];

        for (int i = (int)((gv.length - deta) / 4) * 4; i < gv.length - deta; i++){ // tail case
          tmp = get_pixel(a, x, y + i - ext);
          pc[0] += gv.data[i] / gv.sum[ext - deta] * (*tmp);
          pc[1] += gv.data[i] / gv.sum[ext - deta] * (*(tmp+1)); 
          pc[2] += gv.data[i] / gv.sum[ext - deta] * (*(tmp+2)); 
        }

        pc += 3 * a.dimX;
      }
    }
  return b;
}
*/

// Image apply_gb(Image a, FVec gv) {
//   Image b = gb_h(a, gv);
//   Image c = gb_v(b, gv);
//   free(b.data);
//   return c;
// }

// aux function
void print_fvec(FVec v) {
  unsigned int i;
  printf("\n");
  for (i = 0; i < v.length; i++) {
    printf("%f ", v.data[i]);
  }
  printf("\n");
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

// Image imgOut = apply_gb(img, v);
  Image b = gb_h(img, v);
  Image imgOut = gb_v(b, v);
  // Image imgOut = gb_h(img, v);

  stbi_write_jpg(argv[2], imgOut.dimX, imgOut.dimY, imgOut.numChannels,
                 imgOut.data, 90);

#ifdef FULLRUN
  gettimeofday(&stop_time, NULL);
  timersub(&stop_time, &start_time, &elapsed_time);
  printf("%f \n", elapsed_time.tv_sec + elapsed_time.tv_usec / 1000000.0);
#endif
  free(b.data);
  free(imgOut.data);
  free(v.data);
  free(v.sum);
  free(v.rsum);
  return 0;
}