/*
 * colorbw.c ---- standard colors: black and white
 *
 * Copyright (c) 1998 Hartmut Schirmer
 * Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 * [e-mail: csaba@vuse.vanderbilt.edu]
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
#include "libgrx.h"

/**
 * grx_color_get_black:
 *
 * Gets the color value for black.
 *
 * This is guaranteed to be 0. In C code, you can use #GRX_COLOR_BLACK for short.
 *
 * Returns: the color value.
 */
#ifdef grx_color_get_black
#undef grx_color_get_black
#endif
GrxColor grx_color_get_black(void)
{
        GRX_ENTER();
        if(CLRINFO->black == GRX_COLOR_NONE) CLRINFO->black = grx_color_get(0,0,0);
               GRX_RETURN(CLRINFO->black);
}

/**
 * grx_color_get_white:
 *
 * Gets the color value for white.
 *
 * In C code, you can use #GRX_COLOR_WHITE for short.
 *
 * Returns: the color value.
 */
#ifdef grx_color_get_white
#undef grx_color_get_white
#endif
GrxColor grx_color_get_white(void)
{
        GRX_ENTER();
        if(CLRINFO->white == GRX_COLOR_NONE) CLRINFO->white = grx_color_get(255,255,255);
        GRX_RETURN(CLRINFO->white);
}
