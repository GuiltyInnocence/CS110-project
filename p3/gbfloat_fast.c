#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
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

#define BLOCK 32
#define SIMD 8
#define UNROLL 4

#define unsigned signed

typedef struct FVec {
  unsigned int length;
  unsigned int min_length;
  unsigned int min_deta; //max deta
  float *data;
  float *sum2;
  float *rsum;
} FVec;

typedef struct Image { //RGBRGBRGB
  unsigned int dimX, dimY, numChannels;
  float *data;
} Image;

typedef struct TImg { //Rgraph, G, B
  unsigned int dX, dY;
  float *data;
} TImg;

__inline__ __attribute__((always_inline)) float gd(float a, float b, float x) {
  float c = ((x - b) / a);
  return exp((-.5) * c * c) / (a * sqrt(2 * PI));
}
/*
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
  v.data = malloc((((ext>>3)+1)<<3) * sizeof(float));
  v.sum = malloc((ext + 1) * sizeof(float));
  float step = (x1 - x0) / ((float)length);
  for (int i = 0; i <= ext; i++) 
    v.data[i] = gd(a, 0.0f, (i - ext) * step);
  v.sum[0] = v.data[ext];  // the peak value
  for (i = ext + 1, j = 1; i < v.length; i++, j++) // symmetric
    v.sum[j] = v.sum[j - 1] + v.data[i] * 2;
  return v;
}
*/
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
  v.data = aligned_alloc(256, length * sizeof(float));
  v.sum2 = aligned_alloc(256,(ext + 1) * sizeof(float)); 
  v.rsum = aligned_alloc(256,(ext + 1) * sizeof(float));
  float step = (x1 - x0) / ((float)length);
  for (int i = 0; i < length; i++) 
    v.data[i] = gd(a, 0.0f, (i - ext) * step);
  float sum = v.data[ext]; // the peak value
  v.sum2[0] = v.data[ext];
  v.rsum[0] = 1.00 / v.data[ext];
  for (i = ext + 1, j = 1; i < v.length; i++, j++){ // symmetric
    sum +=  v.data[i] * 2;
    v.sum2[j] = v.sum2[j - 1] + v.data[i];
    v.rsum[j] = 1.00 / sum;
  }
  return v;
}

TImg gb(TImg a, FVec gv){
  float* r1 = aligned_alloc(256,(a.dX * a.dY * 3) * sizeof(float));
  int ext = gv.length >> 1;
#pragma omp parallel for
  for(int yy=0; yy<3*a.dY; ++yy){ //3 channels
    int y=yy % a.dY;
    int ddeta = fmin(a.dY - y - 1, y);
    for(int x=0; x<a.dX; ++x){
      int deta = fmin(fmin(ddeta, gv.min_deta), fmin(a.dX - x - 1, x));
      float sum = 0.0;
      int beg=deta, end=gv.length-deta, tmp=ext-deta;
      // for(int i=deta; x+i-ext<0;i++)sum+=gv.data[i]*a.data[yy*a.dX];
      if(x < tmp){
        sum += (gv.sum2[tmp]-gv.sum2[x])*(a.data[yy*a.dX]);
        beg = ext-x;
      }
      // for(int i=gv.length-deta-1; x+i-ext>=a.dX;i--)sum+=gv.data[i]*a.data[yy*a.dX+a.dX-1];
      if(tmp >= a.dX-x){
        sum += (gv.sum2[tmp]-gv.sum2[a.dX-x-1])*(a.data[yy*a.dX+a.dX-1]);
        end = ext+a.dX-x;
      }
      // for(int k=beg; k<end; k++)sum+=gv.data[k]*(a.data[yy*a.dX+x+k-ext]);
      int k=beg;
      __m256 parts[UNROLL] = {};
      for(; k<=end-SIMD*UNROLL; k+=SIMD*UNROLL){
      // for(; k<=end-SIMD; k+=SIMD){  
        // det = k-ext
        /*__m256 c[UNROLL];
        c[0] = _mm256_loadu_ps(gv.data+k);
        c[1] = _mm256_loadu_ps(gv.data+k+SIMD);
        c[2] = _mm256_loadu_ps(gv.data+k+2*SIMD);
        c[3] = _mm256_loadu_ps(gv.data+k+3*SIMD);
        
        c[0] = _mm256_mul_ps(c[0], _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext));
        c[1] = _mm256_mul_ps(c[1], _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+SIMD));
        c[2] = _mm256_mul_ps(c[2], _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+2*SIMD));
        c[3] = _mm256_mul_ps(c[3], _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+3*SIMD));

        parts[0] = _mm256_add_ps(parts[0], c[0]);
        parts[1] = _mm256_add_ps(parts[1], c[1]);
        parts[2] = _mm256_add_ps(parts[2], c[2]);
        parts[3] = _mm256_add_ps(parts[3], c[3]);
        */
        parts[0] = _mm256_fmadd_ps(_mm256_loadu_ps(gv.data+k), _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext), parts[0]);
        parts[1] = _mm256_fmadd_ps(_mm256_loadu_ps(gv.data+k+SIMD), _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+SIMD), parts[1]);
        parts[2] = _mm256_fmadd_ps(_mm256_loadu_ps(gv.data+k+2*SIMD), _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+2*SIMD), parts[2]);
        parts[3] = _mm256_fmadd_ps(_mm256_loadu_ps(gv.data+k+3*SIMD), _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext+3*SIMD), parts[3]);

      }
      parts[0] = _mm256_add_ps(parts[0], parts[1]);
      parts[2] = _mm256_add_ps(parts[2], parts[3]);
      parts[0] = _mm256_add_ps(parts[0], parts[2]);

      // for(int i=0;i<SIMD;i++)sum+=*((float*)parts+i);
      sum+=*((float*)parts);
      sum+=*((float*)parts+1);
      sum+=*((float*)parts+2);
      sum+=*((float*)parts+3);
      sum+=*((float*)parts+4);
      sum+=*((float*)parts+5);
      sum+=*((float*)parts+6);
      sum+=*((float*)parts+7);
      // for(int i=0;i<UNROLL*SIMD;++i) sum+=*((float*)parts+i);
      /*
      __m256 ret = _mm256_setzero_ps();
      for(; k<=end-SIMD; ++k){
        ret = _mm256_fmadd_ps(_mm256_loadu_ps(gv.data+k), _mm256_loadu_ps(a.data+yy*a.dX+x+k-ext), ret);
      }
      sum+=*((float*)&ret);
      sum+=*((float*)&ret+1);
      sum+=*((float*)&ret+2);
      sum+=*((float*)&ret+3);
      sum+=*((float*)&ret+4);
      sum+=*((float*)&ret+5);
      sum+=*((float*)&ret+6);
      sum+=*((float*)&ret+7);*/
      for(;k<end;k++)sum+=gv.data[k]*(a.data[yy*a.dX+x+k-ext]);
      // r1[yy*a.dX+x] = sum * gv.rsum[ext - deta];
      float tmp2=sum * gv.rsum[ext - deta];
      _mm_stream_si32((int*)(r1+yy*a.dX+x), *((int*)&tmp2));
      // printf("%f\n", r1[yy*a.dX+x]);
    }
  }
  return (TImg){a.dX, a.dY, r1};
}

TImg transpose(TImg a){
  float* r1 = aligned_alloc(256,(a.dX * a.dY * 3) * sizeof(float));
  int siz = a.dX * a.dY;
#pragma omp parallel for collapse(2)
  for(int bx=0; bx<a.dX; bx+=BLOCK){
    for(int by=0; by<a.dY; by+=BLOCK){
      for(int x=bx; x<bx+BLOCK && x<a.dX; ++x){
        for(int y=by; y<by+BLOCK && y<a.dY; ++y){
          r1[x*a.dY+y] = a.data[y*a.dX+x];
          r1[x*a.dY+y+siz] = a.data[y*a.dX+x+siz];
          r1[x*a.dY+y+siz+siz] = a.data[y*a.dX+x+siz+siz];
          // _mm_stream_si32((int*)(r1+x*a.dY+y), *((int*)(a.data+y*a.dX+x)));
          // _mm_stream_si32((int*)(r1+x*a.dY+y+siz), *((int*)(a.data+y*a.dX+x+siz)));
          // _mm_stream_si32((int*)(r1+x*a.dY+y+siz+siz), *((int*)(a.data+y*a.dX+x+siz+siz)));
        }
      }
    }
  }/*
  for(int x=0; x<a.dX; ++x){
    for(int y=0; y<a.dY; ++y){
      r1[x*a.dY+y] = a.data[y*a.dX+x];
      r1[x*a.dY+y+siz] = a.data[y*a.dX+x+siz];
      r1[x*a.dY+y+siz+siz] = a.data[y*a.dX+x+siz+siz];
    }
  }*/
  return (TImg){a.dY, a.dX, r1};
}

Image interpolation(TImg a){
  float* r1 = aligned_alloc(256,(a.dX * a.dY * 3) * sizeof(float));
  int siz = a.dX * a.dY;/*
  float* pc = r1;
  int n1=0;
  int n2=siz;
  int n3=siz<<1;
#pragma gcc unroll(4)
  for(int i=0;i<siz; ++i){
    *(pc++) = a.data[n1++];
    *(pc++) = a.data[n2++];
    *(pc++) = a.data[n3++];
  }*/

#pragma omp parallel for
  for(int y=0; y<a.dY; ++y){
    for(int x=0; x<a.dX; ++x){
      // *(pc++) = a.data[y*a.dX+x];
      // *(pc++) = a.data[y*a.dX+x+siz];
      // *(pc++) = a.data[y*a.dX+x+siz+siz];
      r1[(x+y*a.dX)*3] = a.data[y*a.dX+x];
      r1[(x+y*a.dX)*3+1] = a.data[y*a.dX+x+siz];
      r1[(x+y*a.dX)*3+2] = a.data[y*a.dX+x+siz+siz];
      // _mm_stream_si32((int*)(r1+(x+y*a.dX)*3), *((int*)(a.data+y*a.dX+x)));
      // _mm_stream_si32((int*)(r1+(x+y*a.dX)*3+1), *((int*)(a.data+y*a.dX+x+siz)));
      // _mm_stream_si32((int*)(r1+(x+y*a.dX)*3+2), *((int*)(a.data+y*a.dX+x+siz+siz)));
    }
  }
  return (Image){a.dX, a.dY, 3, r1};
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
      printf("%3.0f %3.0f %3.0f | ",img.data[(i*img.dimX+j)*3],img.data[(i*img.dimX+j)*3+1],img.data[(i*img.dimX+j)*3+2]);
    puts("");
  }
*/
///////////////////////////////////////////////////////////////////////////////////////
  TImg expand; // R,G,B expansion
  expand.dX = img.dimX;
  expand.dY = img.dimY;
  expand.data = aligned_alloc(256,(expand.dX * expand.dY * 3) * sizeof(float));
  int siz = expand.dX * expand.dY;/*
  int n1=0;
  int n2=siz;
  int n3=siz<<1;
  float* pc = img.data;
#pragma gcc unroll(4)
  for(int i=0; i<siz; ++i){
    expand.data[n1++] = *(pc++);
    expand.data[n2++] = *(pc++);
    expand.data[n3++] = *(pc++);
  }*/

#pragma omp parallel for 
  for(int i=0; i<expand.dY; ++i) {
    for(int j=0; j<expand.dX; ++j){
      // *(pc++) = img.data[(i*img.dimX+j)*3];
      // *(pc++) = img.data[(i*img.dimX+j)*3+1];
      // *(pc++) = img.data[(i*img.dimX+j)*3+2];
      expand.data[i*expand.dX+j] = img.data[(i*img.dimX+j)*3];
      expand.data[i*expand.dX+j+siz] = img.data[(i*img.dimX+j)*3+1];
      expand.data[i*expand.dX+j+siz+siz] = img.data[(i*img.dimX+j)*3+2];
      // _mm_stream_si32((int*)(expand.data+i*expand.dX+j), *((int*)(img.data+(i*img.dimX+j)*3)));
      // _mm_stream_si32((int*)(expand.data+i*expand.dX+j+siz), *((int*)(img.data+(i*img.dimX+j)*3+1)));
      // _mm_stream_si32((int*)(expand.data+i*expand.dX+j+siz+siz), *((int*)(img.data+(i*img.dimX+j)*3+2)));
    }
  }

///////////////////////////////////////////////////////////////////////////////////////
/*
  for(int i=0;i<img.dimX;i++){
    for(int j=0;j<img.dimY;j++)
      printf("%3.0f %3.0f %3.0f | ",expand.data[(i*img.dimX+j)*3],expand.data[(i*img.dimX+j)*3+1],expand.data[(i*img.dimX+j)*3+2]);
    puts("");
  }*/

  TImg b = gb(expand, v);
  // puts("alive");
  TImg c = transpose(b);
  // puts("dead");
  TImg d = gb(c, v);
  // puts("awake");
  TImg e = transpose(d);
  // puts("howing");
  Image imgOut = interpolation(e);
/*
  for(int i=0;i<img.dimX;i++){
    for(int j=0;j<img.dimY;j++)
      printf("%3.0f %3.0f %3.0f | ",imgOut.data[(i*img.dimX+j)*3],imgOut.data[(i*img.dimX+j)*3+1],imgOut.data[(i*img.dimX+j)*3+2]);
    puts("");
  }*/
  // puts("amen");
  stbi_write_jpg(argv[2], imgOut.dimX, imgOut.dimY, imgOut.numChannels,
                 imgOut.data, 90);
  // puts("done");
#ifdef FULLRUN
  gettimeofday(&stop_time, NULL);
  timersub(&stop_time, &start_time, &elapsed_time);
  printf("%f \n", elapsed_time.tv_sec + elapsed_time.tv_usec / 1000000.0);
#endif
  free(expand.data);
  free(b.data);
  free(c.data);
  free(d.data);
  free(e.data);
  free(img.data);
  free(imgOut.data);
  free(v.data);
  free(v.rsum);
  free(v.sum2);
  return 0;
}

