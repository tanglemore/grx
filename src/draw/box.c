/*
 * box.c ---- draw a rectangle
 *
 * Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 * [e-mail: csaba@vuse.vanderbilt.edu].
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

#include "globals.h"
#include "mouse.h"
#include "libgrx.h"
#include "clipping.h"

/**
 * grx_draw_box:
 * @x1: the left X coordinate
 * @y1: the top Y coordinate
 * @x2: the right X coordinate
 * @y2: the bottom Y coordinate
 * @c: the color
 *
 * Draws a rectangle on the current context using the specified coordinates
 * and color.
 */
void grx_draw_box(int x1,int y1,int x2,int y2,GrxColor c)
{
        int ox1,oy1,ox2,oy2;
        isort(x1,x2); ox1 = x1; ox2 = x2;
        isort(y1,y2); oy1 = y1; oy2 = y2;
        clip_ordbox(CURC,x1,y1,x2,y2);
        mouse_block(CURC,x1,y1,x2,y2);
        if(!(oy1 -= y1)) (*FDRV->drawhline)(
            x1 + CURC->x_offset,
            y1 + CURC->y_offset,
            x2 - x1 + 1,
            c
        );
        if(!(oy2 -= y2) && ((y1 != y2) || oy1)) (*FDRV->drawhline)(
            x1 + CURC->x_offset,
            y2 + CURC->y_offset,
            x2 - x1 + 1,
            c
        );
        if((y2 = y2 - (oy1 ? y1 : ++y1) + (oy2 ? 1 : 0)) > 0) {
            if(!(ox1 -= x1)) (*FDRV->drawvline)(
                x1 + CURC->x_offset,
                y1 + CURC->y_offset,
                y2,
                c
            );
            if((ox2 == x2) && ((x1 != x2) || ox1)) (*FDRV->drawvline)(
                x2 + CURC->x_offset,
                y1 + CURC->y_offset,
                y2,
                c
            );
        }
        mouse_unblock();
}
