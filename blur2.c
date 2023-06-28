#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define FNAME_IN "in.pgm"
#define FNAME_OUT "out.pgm"
#define PGM_MAX 65535        // Max value of output pgm file 

double *read_pgm_file (const char*, int*, int*);
void write_pgm_file (const char*, const double*, int, int);
double get_current_time (void);

int main (int argc, char** argv) {
  char* fname_in = FNAME_IN;   // input file name
  int b = 1000;                // bokashi level
  char* fname_out = FNAME_OUT; // output file name
  if (argc>=2) {
    fname_in = argv[1];
  }
  if (argc>=3) {
    b = atoi (argv[2]);
  }
  if (argc>=4) {
    fname_out = argv[3];
  }
  int sx, sy;
  double *pixels = read_pgm_file (fname_in, &sx, &sy);
  if (pixels == 0) {
    fprintf (stderr, "Failed to read pgm image.\n");
    exit (1);
  }
  double (*pxout)[sx] = (double(*)[sx]) pixels;
  double (*pxin)[sx] = (double(*)[sx]) malloc (sizeof(double)*sx*sy);
  // => pxin[py][px], pxout[py][px] indicates pixel (px,py)
 
  // Apply filter
  printf ("Apply Gaussian filter %d times.\n", b);
  double t1 = get_current_time();
  for (int t=0; t<b; t++) {
    double (*temp)[sx];
    temp = pxin; pxin = pxout; pxout = temp;
    for (int y=0; y<sy; y++) {
      for (int x=0; x<sx; x++) {
        int y1 = (y-1)<0?0:y-1;
        int y2 = (y+1)>=sy?sy-1:y+1;
        int x1 = (x-1)<0?0:x-1;
        int x2 = (x+1)>=sx?sx-1:x+1;
        pxout[y][x] = 
          pxin[y1][x1]  /16.0
        + pxin[y1][x]   /8.0
        + pxin[y1][x2]  /16.0
        + pxin[y][x1]   /8.0
        + pxin[y][x]    /4.0
        + pxin[y][x2]   /8.0
        + pxin[y2][x1]  /16.0
        + pxin[y2][x]   /8.0
        + pxin[y2][x2]  /16.0;
      }
    }
  }
  double t2 = get_current_time();
  printf ("Finished. Time: %lf seconds.\n", t2-t1);

  write_pgm_file (fname_out, (double*)pxout, sx, sy);
  printf ("'eog %s' to check the result image.\n", fname_out);

  free (pxin); free (pxout);
  return 0;
}

// Load P5 (pgm) image file
// fname: file name
// Picture size (sx,sy) is output to *p_sx and *p_sy.
// Pointer to pixel data (double[sx*sy]) is returned.
double *read_pgm_file (const char* fname, int *p_sx, int *p_sy)
{
  FILE *fp = fopen (fname, "rb");
  if (fp == NULL) {
    fprintf (stderr, "Failed to open file %s\n", fname);
    return 0;
  }
  char f1, f2;
  // Check format letter "P5"
  int n = fscanf(fp, "%c%c", &f1, &f2);
  if ( n<2 || f1 != 'P' || f2 != '5') {
      fprintf(stderr, "Invalid PGM format.\n");
      fclose(fp);
      return 0;
  }
  // Read size and max value
  int sx, sy, pgm_max;
  int bpp;               // # bytes per pixel
  n = fscanf (fp, "%d%d%d", &sx, &sy, &pgm_max);
  if ( n<3 ) {
      fprintf(stderr, "Invalid PGM format.\n");
      fclose(fp);
      return 0;
  }
  if (pgm_max > 65535) {
    fprintf(stderr, "Invalid Maxval: %d. (should be less than 65536)\n", pgm_max);
    fclose(fp);
    return 0;
  }
  bpp = (pgm_max>255)? 2: 1;
  fgetc(fp); // Read the newline character before pixel data
  // Read pixel data
  double *pixels = (double*) malloc(sizeof(double)*sx*sy);
  int i=0;
  for (int y=0; y<sy; y++) {
    for (int x=0; x<sx; x++) {
        unsigned int c;
        int n = fread (&c, bpp, 1, fp);
        if (n<1) {
          fprintf (stderr, "Failed to read pixel data\n");
          free (pixels);
          fclose (fp);
          return 0;
        }
        double c_d = (double)c/pgm_max;
        pixels[i++] = c_d;
        // printf ("%u(%f)", c, c_d);
    }
    // printf ("\n");
  }
  fclose (fp);
  *p_sx = sx; *p_sy = sy;
  return pixels;
}

// Output P5 (pgm) image file
// fname: file name
// pixels: pixel data of size sx*sy
void write_pgm_file (const char* fname, const double *pixels, int sx, int sy)
{
  FILE *fp = fopen (fname, "wb");
  int bpp = (PGM_MAX>255)? 2: 1;    // # bytes per pixel
  double (*pxout)[sx] = (double(*)[sx]) pixels;
  fprintf (fp, "P5\n%u %u\n%u\n", sx, sy, PGM_MAX);
  for (int y=0; y<sy; y++) {
    for (int x=0; x<sx; x++) {
        double px = pxout[y][x]*PGM_MAX;
        unsigned int c = (px>PGM_MAX)?PGM_MAX:(px<0)?0:px;
        // printf ("%u ", c);
        fwrite((char*)(&c)+(bpp-1), bpp, 1, fp);        
    }
  }
  fclose (fp);
}

// 現在時刻を取得する．精度はナノ秒（ns），返す値の単位は秒（s）
double get_current_time (void)
{
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  return (tp.tv_sec+(double)tp.tv_nsec/1000/1000/1000);
}
