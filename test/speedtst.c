/*
 * speedtst.c ---- check all available frame drivers speed
 *
 * Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 * [e-mail: csaba@vuse.vanderbilt.edu]
 *
 * This is a test/demo file of the GRX graphics library.
 * You can use GRX test/demo files as you want.
 *
 * The GRX graphics library is free software; you can redistribute it
 * and/or modify it under some conditions; see the "copying.grx" file
 * for details.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * 070512 M.Alvarez, new version more accurate, but still had problems
 *                   in X11, because functions returns before the paint
 *                   is done.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <values.h>
#include <math.h>
#include "rand.h"

#include "grx-3.0.h"

#define BLIT_FAIL(gp)  0

#define MEASURE_RAM_MODES 1

#define READPIX_loops      (384*1)
#define READPIX_X11_loops  (4*1)
#define DRAWPIX_loops      (256*1)
#define DRAWLIN_loops      (12*1)
#define DRAWHLIN_loops     (16*1)
#define DRAWVLIN_loops     (12*1)
#define DRAWBLK_loops      (1*1)
#define BLIT_loops         (1*1)

typedef struct {
    double rate, count;
} perfm;

typedef struct {
    GrxFrameMode fm;
    int    w,h,bpp;
    int    flags;
    perfm  readpix;
    perfm  drawpix;
    perfm  drawlin;
    perfm  drawhlin;
    perfm  drawvlin;
    perfm  drawblk;
    perfm  blitv2v;
    perfm  blitv2r;
    perfm  blitr2v;
} gvmode;
#define FLG_measured 0x0001
#define FLG_tagged   0x0002
#define FLG_rammode  0x0004
#define MEASURED(g) (((g)->flags&FLG_measured)!=0)
#define TAGGED(g)   (((g)->flags&FLG_tagged)!=0)
#define RAMMODE(g)  (((g)->flags&FLG_rammode)!=0)
#define SET_MEASURED(g)  (g)->flags |= FLG_measured
#define SET_TAGGED(g)    (g)->flags |= FLG_tagged
#define SET_RAMMODE(g)   (g)->flags |= FLG_rammode
#define TOGGLE_TAGGED(g) (g)->flags ^= FLG_tagged

int  n_modes = 0;
#define MAX_MODES 256
gvmode *grmodes = NULL;
#if MEASURE_RAM_MODES
gvmode *rammodes = NULL;
#endif

static GrxTextOptions *text_opt;

/* No of Points [(x,y) pairs]. Must be multiple of 2*3=6 */
#define PAIRS 4200

#define UL(x)  ((unsigned long)(x))
#define DBL(x)  ((double)(x))
#define INT(x) ((int)(x))

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

typedef struct XYpairs {
  int x[PAIRS];
  int y[PAIRS];
  int w, h;
  struct XYpairs *nxt;
} XY_PAIRS;

XY_PAIRS *xyp = NULL;
int *xb = NULL, *yb = NULL; /* need sorted pairs for block operations */
int measured_any = 0;

XY_PAIRS *checkpairs(int w, int h) {
  XY_PAIRS *res = xyp;
  int i;

  if (xb == NULL) {
    xb = malloc(sizeof(int) * PAIRS);
    yb = malloc(sizeof(int) * PAIRS);
  }

  while (res != NULL) {
    if (res->w == w && res->h == h)
      return res;
    res = res->nxt;
  }

  SRND(12345);

  res = malloc(sizeof(XY_PAIRS));
  assert(res != NULL);
  res->w = w;
  res->h = h;
  res->nxt = xyp;
  xyp = res;
  for (i=0; i < PAIRS; ++i) {
    int x = RND() % w;
    int y = RND() % h;
    if (x < 0) x = 0; else
    if (x >=w) x = w-1;
    if (y < 0) y = 0; else
    if (y >=h) y = h-1;
    res->x[i] = x;
    res->y[i] = y;
  }
  return res;
}

double SQR(int a, int b) {
  double r = DBL(a-b);
  return r*r;
}

double abs_diff(int a, int b) {
  double r = DBL(a-b);
  return fabs(r);
}

char *FrameDriverName(GrxFrameMode m) {

  switch(m) {
    case GRX_FRAME_MODE_UNDEFINED: return "Undef";
    case GRX_FRAME_MODE_TEXT : return "Text";
    case GRX_FRAME_MODE_LFB_MONO01: return "LFB1-01";
    case GRX_FRAME_MODE_LFB_MONO10: return "LFB1-10";
    case GRX_FRAME_MODE_LFB_2BPP: return "LFB2";
    case GRX_FRAME_MODE_LFB_8BPP: return "LFB8";
    case GRX_FRAME_MODE_LFB_16BPP: return "LFB16";
    case GRX_FRAME_MODE_LFB_24BPP: return "LFB24";
    case GRX_FRAME_MODE_LFB_32BPP_LOW: return "LFB32L";
    case GRX_FRAME_MODE_LFB_32BPP_HIGH: return "LFB32H";
    case GRX_FRAME_MODE_RAM_1BPP: return "RAM1";
    case GRX_FRAME_MODE_RAM_2BPP: return "RAM2";
    case GRX_FRAME_MODE_RAM_4X1BPP: return "RAM4";
    case GRX_FRAME_MODE_RAM_8BPP: return "RAM8";
    case GRX_FRAME_MODE_RAM_16BPP: return "RAM16";
    case GRX_FRAME_MODE_RAM_24BPP: return "RAM24";
    case GRX_FRAME_MODE_RAM_32BPP_LOW: return "RAM32L";
    case GRX_FRAME_MODE_RAM_32BPP_HIGH: return "RAM32H";
    case GRX_FRAME_MODE_RAM_3X8BPP: return "RAM3x8";
    default: return "UNKNOWN";
  }
  return "UNKNOWN";
}

void Message(int disp, char *txt, gvmode *gp) {
  char msg[200];
  sprintf(msg, "%s: %d x %d x %dbpp",
                FrameDriverName(gp->fm), gp->w, gp->h, gp->bpp);

  if (disp) {
    GrxContext save;

    grx_save_current_context(&save);
    grx_set_current_context(NULL);
    grx_draw_text(msg, 0, 0, text_opt);
    grx_draw_text(txt, 0, 10, text_opt);
    grx_set_current_context(&save);
  }
}

void printresultheader(FILE *f) {
  fprintf(f, "Driver               readp drawp line   hline vline  block  v2v    v2r    r2v\n");
}

void printresultline(FILE *f, gvmode * gp) {
  fprintf(f, "%-9s %4dx%4d ", FrameDriverName(gp->fm), gp->w, gp->h);
  fprintf(f, "%6.2f", gp->readpix.rate  / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawpix.rate  / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawlin.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->drawhlin.rate / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawvlin.rate / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->drawblk.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitv2v.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitv2r.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitr2v.rate  / (1024.0 * 1024.0));
  fprintf(f, "\n");
}

void readpixeltest(gvmode *gp, XY_PAIRS *pairs,int loops) {
  int i, j;
  long t1,t2;
  double seconds;
  int *x = pairs->x;
  int *y = pairs->y;

  if (!MEASURED(gp)) {
    gp->readpix.rate  = 0.0;
    gp->readpix.count = DBL(PAIRS) * DBL(loops);
  }

  t1 = g_get_monotonic_time();
  for (i=loops; i > 0; --i) {
    for (j=PAIRS-1; j >= 0; j--)
       grx_fast_get_pixel_at(x[j],y[j]);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->readpix.rate = gp->readpix.count / seconds;
}

void drawpixeltest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  GrxColor c1 = GRX_COLOR_WHITE;
  GrxColor c2 = GRX_COLOR_WHITE | GRX_COLOR_MODE_XOR;
  GrxColor c3 = GRX_COLOR_WHITE | GRX_COLOR_MODE_OR;
  GrxColor c4 = GRX_COLOR_BLACK | GRX_COLOR_MODE_AND;
  long t1,t2;
  double seconds;
  int *x = pairs->x;
  int *y = pairs->y;

  if (!MEASURED(gp)) {
    gp->drawpix.rate  = 0.0;
    gp->drawpix.count = DBL(PAIRS) * DBL(DRAWPIX_loops) * 4.0;
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < DRAWPIX_loops; ++i) {
    for (j=PAIRS-1; j >= 0; j--) grx_fast_draw_pixel(x[j],y[j],c1);
    for (j=PAIRS-1; j >= 0; j--) grx_fast_draw_pixel(x[j],y[j],c2);
    for (j=PAIRS-1; j >= 0; j--) grx_fast_draw_pixel(x[j],y[j],c3);
    for (j=PAIRS-1; j >= 0; j--) grx_fast_draw_pixel(x[j],y[j],c4);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->drawpix.rate = gp->drawpix.count / seconds;
}

void drawlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrxColor c1 = GRX_COLOR_WHITE;
  GrxColor c2 = GRX_COLOR_WHITE | GRX_COLOR_MODE_XOR;
  GrxColor c3 = GRX_COLOR_WHITE | GRX_COLOR_MODE_OR;
  GrxColor c4 = GRX_COLOR_BLACK | GRX_COLOR_MODE_AND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawlin.rate  = 0.0;
    gp->drawlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawlin.count += sqrt(SQR(x[j],x[j+1])+SQR(y[j],y[j+1]));
    gp->drawlin.count *= 4.0 * DRAWLIN_loops;
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < DRAWLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
        grx_fast_draw_line(x[j],y[j],x[j+1],y[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
        grx_fast_draw_line(x[j],y[j],x[j+1],y[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
        grx_fast_draw_line(x[j],y[j],x[j+1],y[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
        grx_fast_draw_line(x[j],y[j],x[j+1],y[j+1],c4);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->drawlin.rate = gp->drawlin.count / seconds;
}

void drawhlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int  i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrxColor c1 = GRX_COLOR_WHITE;
  GrxColor c2 = GRX_COLOR_WHITE | GRX_COLOR_MODE_XOR;
  GrxColor c3 = GRX_COLOR_WHITE | GRX_COLOR_MODE_OR;
  GrxColor c4 = GRX_COLOR_BLACK | GRX_COLOR_MODE_AND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawhlin.rate = 0.0;
    gp->drawhlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawhlin.count += abs_diff(x[j],x[j+1]);
    gp->drawhlin.count *= 4.0 * DRAWHLIN_loops;
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < DRAWHLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_hline(x[j],x[j+1],y[j],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_hline(x[j],x[j+1],y[j],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_hline(x[j],x[j+1],y[j],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_hline(x[j],x[j+1],y[j],c4);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->drawhlin.rate = gp->drawhlin.count / seconds;
}

void drawvlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrxColor c1 = GRX_COLOR_WHITE;
  GrxColor c2 = GRX_COLOR_WHITE | GRX_COLOR_MODE_XOR;
  GrxColor c3 = GRX_COLOR_WHITE | GRX_COLOR_MODE_OR;
  GrxColor c4 = GRX_COLOR_BLACK | GRX_COLOR_MODE_AND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawvlin.rate = 0.0;
    gp->drawvlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawvlin.count += abs_diff(y[j],y[j+1]);
    gp->drawvlin.count *= 4.0 * DRAWVLIN_loops;
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < DRAWVLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
       grx_fast_draw_vline(x[j],y[j],y[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
       grx_fast_draw_vline(x[j],y[j],y[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
       grx_fast_draw_vline(x[j],y[j],y[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
       grx_fast_draw_vline(x[j],y[j],y[j+1],c4);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->drawvlin.rate = gp->drawvlin.count / seconds;
}

void drawblocktest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  GrxColor c1 = GRX_COLOR_WHITE;
  GrxColor c2 = GRX_COLOR_WHITE | GRX_COLOR_MODE_XOR;
  GrxColor c3 = GRX_COLOR_WHITE | GRX_COLOR_MODE_OR;
  GrxColor c4 = GRX_COLOR_BLACK | GRX_COLOR_MODE_AND;
  long t1,t2;
  double seconds;

  if (xb == NULL || yb == NULL) return;

  for (j=0; j < PAIRS; j+=2) {
    xb[j]   = min(pairs->x[j],pairs->x[j+1]);
    xb[j+1] = max(pairs->x[j],pairs->x[j+1]);
    yb[j]   = min(pairs->y[j],pairs->y[j+1]);
    yb[j+1] = max(pairs->y[j],pairs->y[j+1]);
  }

  if (!MEASURED(gp)) {
    gp->drawblk.rate = 0.0;
    gp->drawblk.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawblk.count += abs_diff(xb[j],xb[j+1]) * abs_diff(yb[j],yb[j+1]);
    gp->drawblk.count *= 4.0 * DRAWBLK_loops;
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < DRAWBLK_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_filled_box(xb[j],yb[j],xb[j+1],yb[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_filled_box(xb[j],yb[j],xb[j+1],yb[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_filled_box(xb[j],yb[j],xb[j+1],yb[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
      grx_fast_draw_filled_box(xb[j],yb[j],xb[j+1],yb[j+1],c4);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    gp->drawblk.rate = gp->drawblk.count / seconds;
}

void xor_draw_blocks(GrxContext *c) {
  GrxContext save;
  int i;

  grx_save_current_context(&save);
  grx_set_current_context(c);
  grx_clear_context(GRX_COLOR_BLACK);
  for (i=28; i > 1; --i)
    grx_draw_filled_box(grx_get_max_x()/i,grx_get_max_y()/i,
                (i-1)*grx_get_max_x()/i,(i-1)*grx_get_max_y()/i,GRX_COLOR_WHITE|GRX_COLOR_MODE_XOR);
  grx_set_current_context(&save);
}

void blit_measure(gvmode *gp, perfm *p,
                  int *xb, int *yb,
                  GrxContext *dst,GrxContext *src) {
  int i, j;
  long t1,t2;
  double seconds;
  GrxContext save;

  grx_save_current_context(&save);
  if (dst != src) {
    grx_set_current_context(dst);
    grx_clear_context(GRX_COLOR_BLACK);
  }
  xor_draw_blocks(src);
  grx_set_current_context(&save);

  if (dst != NULL) {
    char *s = src != NULL ? "ram" : "video";
    char *d = dst != NULL ? "ram" : "video";
    char txt[50];
    sprintf(txt, "blit test: %s -> %s", s, d);
    Message(1,txt, gp);
  }

  t1 = g_get_monotonic_time();
  for (i=0; i < BLIT_loops; ++i) {
    for (j=PAIRS-3; j >= 0; j-=3)
      grx_context_bit_blt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GRX_COLOR_MODE_WRITE);
    for (j=PAIRS-3; j >= 0; j-=3)
      grx_context_bit_blt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GRX_COLOR_MODE_XOR);
    for (j=PAIRS-3; j >= 0; j-=3)
      grx_context_bit_blt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GRX_COLOR_MODE_OR);
    for (j=PAIRS-3; j >= 0; j-=3)
      grx_context_bit_blt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GRX_COLOR_MODE_AND);
  }
  t2 = g_get_monotonic_time();
  seconds = (double)(t2 - t1) / 1000000.0;
  if (seconds > 0)
    p->rate = p->count / seconds;
}

void blittest(gvmode *gp, XY_PAIRS *pairs, int ram) {
  int j;

  if (xb == NULL || yb == NULL) return;

  for (j=0; j < PAIRS; j+=3) {
    int wh;
    xb[j]   = max(pairs->x[j],pairs->x[j+1]);
    xb[j+1] = min(pairs->x[j],pairs->x[j+1]);
    xb[j+2] = pairs->x[j+2];
    wh      = xb[j]-xb[j+1];
    if (xb[j+2]+wh >= gp->w) xb[j+2] = gp->w - wh - 1;
    yb[j]   = max(pairs->y[j],pairs->y[j+1]);
    yb[j+1] = min(pairs->y[j],pairs->y[j+1]);
    yb[j+2] = pairs->y[j+2];
    wh      = yb[j]-yb[j+1];
    if (yb[j+2]+wh >= gp->h) yb[j+2] = gp->h - wh - 1;
  }

  if (!MEASURED(gp)) {
    double count = 0.0;
    for (j=0; j < PAIRS; j+=3)
      count += abs_diff(xb[j],xb[j+1]) * abs_diff(yb[j],yb[j+1]);
    gp->blitv2v.count =
    gp->blitr2v.count =
    gp->blitv2r.count = count * 4.0 * BLIT_loops;
    gp->blitv2v.rate  =
    gp->blitr2v.rate  =
    gp->blitv2r.rate  = 0.0;
  }

#if BLIT_loops-0
  blit_measure(gp, &gp->blitv2v, xb, yb,
               (GrxContext *)(RAMMODE(gp) ? grx_get_current_context() : NULL),
               (GrxContext *)(RAMMODE(gp) ? grx_get_current_context() : NULL));
  if (!BLIT_FAIL(gp) && !ram) {
    GrxContext rc;
    GrxContext *rcp = grx_context_new(gp->w,gp->h,NULL,&rc);
    if (rcp) {
      blit_measure(gp, &gp->blitv2r, xb, yb, rcp, NULL);
      blit_measure(gp, &gp->blitr2v, xb, yb, NULL, rcp);
      grx_context_unref(rcp);
    }
  }
#endif
}

void measure_one(gvmode *gp, int ram) {
  XY_PAIRS *pairs;

  if (MEASURED(gp)) return;
  pairs = checkpairs(gp->w, gp->h);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"read pixel test", gp);

  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"draw pixel test", gp);
  drawpixeltest(gp,pairs);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"draw line test ", gp);
  drawlinetest(gp,pairs);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"draw hline test", gp);
  drawhlinetest(gp,pairs);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"draw vline test", gp);
  drawvlinetest(gp,pairs);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  Message(RAMMODE(gp),"draw block test", gp);
  drawblocktest(gp,pairs);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  blittest(gp, pairs, ram);
  grx_draw_filled_box( 0, 0, gp->w-1, gp->h-1, GRX_COLOR_BLACK);
  SET_MEASURED(gp);
  measured_any = 1;
}

#if MEASURE_RAM_MODES
int identical_measured(gvmode *tm) {
  int i;
  for (i=0; i < n_modes; ++i) {
    if (tm      != &rammodes[i]    &&
        tm->fm  == rammodes[i].fm  &&
        tm->w   == rammodes[i].w   &&
        tm->h   == rammodes[i].h   &&
        tm->bpp == rammodes[i].bpp &&
        MEASURED(&rammodes[i])        ) return (1);
  }
  return 0;
}
#endif

static int first = 0;

void speedcheck(gvmode *gp, int print, int wait) {
  char m[41];
  gvmode *rp = NULL;

  if (first) {
    printf(
      "speedtest may take some time to process.\n"
      "Now press <CR> to continue..."
    );
    fflush(stdout);
    fgets(m,40,stdin);
  }

  grx_set_mode(
      GRX_GRAPHICS_MODE_GRAPHICS_WIDTH_HEIGHT_BPP, NULL,
      gp->w, gp->h, gp->bpp
  );

  if (first) {
    /* xor_draw_blocks(NULL);
       getch(); */
    first = 0;
  }

  if ( grx_frame_mode_get_screen() != gp->fm) {
    GrxFrameMode act = grx_frame_mode_get_screen();
    grx_set_mode(GRX_GRAPHICS_MODE_TEXT_DEFAULT, NULL);
    printf("Setup failed : %s != %s\n",
    FrameDriverName(act),
    FrameDriverName(gp->fm));
    fgets(m,40,stdin);
    return;
  }

  if (!MEASURED(gp))
    measure_one(gp, 0);

#if MEASURE_RAM_MODES
  rp = &rammodes[(unsigned)(gp-grmodes)];
  rp->fm = grx_frame_mode_get_screen_core();
  if (!MEASURED(rp) && !identical_measured(rp)) {
    GrxContext rc;
    if (grx_context_new_full(rp->fm,gp->w,gp->h,NULL,&rc)) {
      grx_set_current_context(&rc);
      measure_one(rp, 1);
      grx_context_unref(&rc);
      grx_set_current_context(NULL);
    }
  }
#endif

  grx_set_mode(GRX_GRAPHICS_MODE_TEXT_DEFAULT, NULL);
  if (print) {
    printf("Results: \n");
    printresultheader(stdout);
    printresultline(stdout, gp);
    if (rp)
      printresultline(stdout, rp);
  }
  if (wait) 
    fgets(m,40,stdin);
}

int collectmodes(const GrxVideoDriver *drv)
{
        gvmode *gp = grmodes;
        GrxFrameMode fm;
        const GrxVideoMode *mp;
        for(fm =GRX_FRAME_MODE_FIRST_GRAPHICS;
              fm <= GRX_FRAME_MODE_LAST_GRAPHICS; fm++) {
            for(mp = grx_get_first_video_mode(fm); mp; mp = grx_get_next_video_mode(mp)) {
                gp->fm    = fm;
                gp->w     = mp->width;
                gp->h     = mp->height;
                gp->bpp   = mp->bpp;
                gp->flags = 0;
                gp++;
                if (gp-grmodes >= MAX_MODES) return MAX_MODES;
            }
        }
        return(int)(gp-grmodes);
}

int vmcmp(const void *m1,const void *m2)
{
        gvmode *md1 = (gvmode *)m1;
        gvmode *md2 = (gvmode *)m2;
        if(md1->bpp != md2->bpp) return(md1->bpp - md2->bpp);
        if(md1->w   != md2->w  ) return(md1->w   - md2->w  );
        if(md1->h   != md2->h  ) return(md1->h   - md2->h  );
        return(0);
}

#define LINES   20
#define COLUMNS 80

void ModeText(int i, int shrt,char *mdtxt) {
        char *flg;

        if (MEASURED(&grmodes[i])) flg = " #"; else
        if (TAGGED(&grmodes[i]))   flg = " *"; else
                                   flg = ") ";
        switch (shrt) {
          case 2 : sprintf(mdtxt,"%2d%s %dx%d ", i+1, flg, grmodes[i].w, grmodes[i].h);
                   break;
          case 1 : sprintf(mdtxt,"%2d%s %4dx%-4d ", i+1, flg, grmodes[i].w, grmodes[i].h);
                   break;
          default: sprintf(mdtxt,"  %2d%s  %4dx%-4d ", i+1, flg, grmodes[i].w, grmodes[i].h);
                   break;
        }
        mdtxt += strlen(mdtxt);

        if (grmodes[i].bpp > 20)
          sprintf(mdtxt, "%ldM", 1L << (grmodes[i].bpp-20));
        else  if (grmodes[i].bpp > 10)
          sprintf(mdtxt, "%ldk", 1L << (grmodes[i].bpp-10));
        else
          sprintf(mdtxt, "%ld", 1L << grmodes[i].bpp);
        switch (shrt) {
          case 2 : break;
          case 1 : strcat(mdtxt, " col"); break;
          default: strcat(mdtxt, " colors"); break;
        }
}

int ColsCheck(int cols, int ml, int sep) {
  int len;

  len = ml * cols + (cols-1) * sep + 1;
  return len <= COLUMNS;
}

void PrintModes(void) {
        char mdtxt[100];
        unsigned int maxlen;
        int i, n, shrt, c, cols;

        cols = (n_modes+LINES-1) / LINES;
        do {
          for (shrt = 0; shrt <= 2; ++shrt) {
            maxlen = 0;
            for (i = 0; i < n_modes; ++i) {
              ModeText(i,shrt,mdtxt);
              if (strlen(mdtxt) > maxlen) maxlen = strlen(mdtxt);
            }
            n = 2;
            if (cols>1 || shrt<2) {
              if (!ColsCheck(cols, maxlen, n)) continue;
              while (ColsCheck(cols, maxlen, n+1) && n < 4) ++n;
            }
            c = 0;
            for (i = 0; i < n_modes; ++i) {
              if (++c == cols) c = 0;
              ModeText(i,shrt,mdtxt);
              printf("%*s%s", (c ? -((int)(maxlen+n)) : -((int)maxlen)), mdtxt, (c || (i+1==n_modes) ? "" : "\n") );
            }
            return;
          }
          --cols;
        } while (1);
}

int main(int argc, char **argv)
{
        GrxFont *font;
        GError *err = NULL;
        int  i;

        font = grx_font_load(NULL, -1, &err);
        if (!font) {
          g_error("%s", err->message);
        }
        text_opt = grx_text_options_new_full(font, GRX_COLOR_WHITE, GRX_COLOR_BLACK,
                                        GRX_TEXT_HALIGN_LEFT, GRX_TEXT_VALIGN_TOP);
        grx_font_unref(font);

        grmodes = malloc(MAX_MODES*sizeof(gvmode));
        assert(grmodes!=NULL);
#if MEASURE_RAM_MODES
        rammodes = malloc(MAX_MODES*sizeof(gvmode));
        assert(rammodes!=NULL);
#endif

        grx_set_driver(NULL, NULL);
        if(grx_get_current_video_driver() == NULL) {
            printf("No graphics driver found\n");
            exit(1);
        }

        n_modes = collectmodes(grx_get_current_video_driver());
        if(n_modes == 0) {
            printf("No graphics modes found\n");
            exit(1);
        }
        qsort(grmodes,n_modes,sizeof(grmodes[0]),vmcmp);
#if MEASURE_RAM_MODES
        for (i=0; i < n_modes; ++i) {
          rammodes[i].fm    = GRX_FRAME_MODE_UNDEFINED;      /* filled in later */
          rammodes[i].w     = grmodes[i].w;
          rammodes[i].h     = grmodes[i].h;
          rammodes[i].bpp   = grmodes[i].bpp;
          rammodes[i].flags = FLG_rammode;
        }
#endif

        if(argc >= 2 && (i = atoi(argv[1])) >= 1 && i <= n_modes) {
            speedcheck(&grmodes[i - 1], 1, 0);
            goto done;
        }

        first = 1;
        for( ; ; ) {
            char mb[41], *m = mb;
            int tflag = 0;
            grx_set_mode(GRX_GRAPHICS_MODE_TEXT_DEFAULT, NULL);
            printf(
                "Graphics driver: \"%s\"\t"
                "graphics defaults: %dx%d %ld colors\n",
                grx_get_current_video_driver()->name,
                GrDriverInfo->defgw,
                GrDriverInfo->defgh,
                (long)GrDriverInfo->defgc
            );
            PrintModes();
            printf("\nEnter #, 't#' toggels tag, 'm' measure tagged and 'q' to quit> ");
            fflush(stdout);
            if(!fgets(m,40,stdin)) break;
            switch (*m) {
              case 't':
              case 'T': tflag = 1;
                        ++m;
                        break;
              case 'A':
              case 'a': for (i=0; i < n_modes; ++i)
                          SET_TAGGED(&grmodes[i]);
                        break;
              case 'M':
              case 'm': for (i=0; i < n_modes; ++i)
                          if (TAGGED(&grmodes[i])) {
                            speedcheck(&grmodes[i], 0, 0);
                            TOGGLE_TAGGED(&grmodes[i]);
                          }
                        break;
              case 'Q':
              case 'q': goto done;
            }
            if ((sscanf(m,"%d",&i) != 1) || (i < 1) || (i > n_modes))
                continue;
            i--;
            if (tflag) TOGGLE_TAGGED(&grmodes[i]);
                  else speedcheck(&grmodes[i], 1, 1);
        }
done:
        if (measured_any) {
            int i;
            FILE *log = fopen("speedtst.log", "a");

            if (!log) exit(1);

            fprintf( log, "\nGraphics driver: \"%s\"\n\n",
                                               grx_get_current_video_driver()->name);
            printf("Results: \n");
            printresultheader(log);

            for (i=0; i < n_modes; ++i)
              if (MEASURED(&grmodes[i]))
                printresultline(log, &grmodes[i]);
#if MEASURE_RAM_MODES
            for (i=0; i < n_modes; ++i)
              if (MEASURED(&rammodes[i]))
                printresultline(log, &rammodes[i]);
#endif
            fclose(log);
        }

        grx_text_options_unref(text_opt);

        return(0);
}
