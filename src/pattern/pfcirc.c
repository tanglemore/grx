/*
 * pfcirc.c
 *
 * Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 * [e-mail: csaba@vuse.vanderbilt.edu]
 *
 *  Copyright (C) 1992, Csaba Biegl
 *    820 Stirrup Dr, Nashville, TN, 37221
 *    csaba@vuse.vanderbilt.edu
 *
 * This file is part of the GRX graphics library.
 *
 * The GRX graphics library is free software; you can redistribute it
 * and/or modify it under some conditions; see the "copying.grx" file
 * for details.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <grx/pixmap.h>

#include "libgrx.h"

/**
 * grx_draw_filled_circle_with_pixmap:
 * @xc: the X coordinate of the center of the circle
 * @yc: the Y coordinate of the center of the circle
 * @r: the radius of the circle
 * @p: the pixmap
 *
 * Draws a filled circle on the current context centered at the specified
 * coordinates with the specified radius and pixmap.
 */
void grx_draw_filled_circle_with_pixmap(int xc,int yc,int r,GrxPixmap *p)
{
        grx_draw_filled_ellipse_with_pixmap(xc,yc,r,r,p);
}
