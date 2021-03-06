/*
 * memtest.c ---- test memory driver
 *
 * Copyright (C) 2001 Mariano Alvarez Fernandez
 * [e-mail: malfer@teleline.es]
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
 */

#include <stdio.h>
#include "grx-3.0.h"
#include "drawing.h"

int main()
{
  int x, y, ww, wh;
  GrxColor c;
  GrxContext *w1, *w2, *w3, *w4;
  GError *error = NULL;

  grx_set_driver("memory gw 400 gh 400 nc 256", NULL);
  if (!grx_set_mode(GRX_GRAPHICS_MODE_GRAPHICS_DEFAULT, &error)) {
    g_error("%s", error->message);
  }

  x = grx_get_width();
  y = grx_get_height();
  ww = (x / 2) - 10;
  wh = (y / 2) - 10;
  w1 = grx_context_new_subcontext(5,5,ww+4,wh+4,NULL,NULL);
  w2 = grx_context_new_subcontext(15+ww,5,ww+ww+14,wh+4,NULL,NULL);
  w3 = grx_context_new_subcontext(5,15+wh,ww+4,wh+wh+14,NULL,NULL);
  w4 = grx_context_new_subcontext(15+ww,15+wh,ww+ww+14,wh+wh+14,NULL,NULL);

  grx_set_current_context(w1);
  c = grx_color_get(200,100,100);
  drawing(0,0,ww,wh,c,GRX_COLOR_BLACK);
  grx_draw_box(0,0,ww-1,wh-1,c);

  grx_set_current_context(w2);
  c = grx_color_get(100,200,200);
  drawing(0,0,ww,wh,c,GRX_COLOR_BLACK);
  grx_draw_box(0,0,ww-1,wh-1,c);

  grx_set_current_context(w3);
  c = grx_color_get(200,200,0);
  drawing(0,0,ww,wh,c,GRX_COLOR_BLACK);
  grx_draw_box(0,0,ww-1,wh-1,c);

  grx_set_current_context(w4);
  c = grx_color_get(0,100,200);
  drawing(0,0,ww,wh,c,GRX_COLOR_BLACK);
  grx_draw_box(0,0,ww-1,wh-1,c);

  grx_set_current_context( NULL );
//  GrSaveBmpImage( "memtest.bmp",NULL,0,0,639,479 );
  grx_context_save_to_ppm( NULL,"memtest.ppm","GRX MemTest",NULL );

  return 0;
}
