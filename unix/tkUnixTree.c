/* 
 * tkUnixTree.c --
 *
 *	Platform-specific parts of TkTreeCtrl for X11.
 *
 * Copyright (c) 2010 Tim Baker
 *
 * RCS: @(#) $Id$
 */

#include "tkTreeCtrl.h"

/*
 *----------------------------------------------------------------------
 *
 * Tree_HDotLine --
 *
 *	Draws a horizontal 1-pixel-tall dotted line.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_HDotLine(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int x1, int y1, int x2	/* Left, top and right coordinates. */
    )
{
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;

    nw = !(wx & 1) == !(wy & 1);
    for (x1 += !nw; x1 < x2; x1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_VDotLine --
 *
 *	Draws a vertical 1-pixel-wide dotted line.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_VDotLine(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int x1, int y1, int y2)	/* Left, top, and bottom coordinates. */
{
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;

    nw = !(wx & 1) == !(wy & 1);
    for (y1 += !nw; y1 < y2; y1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_DrawActiveOutline --
 *
 *	Draws 0 or more sides of a rectangle, dot-on dot-off, XOR style.
 *	This is used by rectangle Elements to indicate the "active"
 *	item.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_DrawActiveOutline(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    int x, int y,		/* Left and top coordinates. */
    int width, int height,	/* Size of rectangle. */
    int open			/* RECT_OPEN_x flags */
    )
{
    int wx = x + tree->drawableXOrigin;
    int wy = y + tree->drawableYOrigin;
    int w = !(open & RECT_OPEN_W);
    int n = !(open & RECT_OPEN_N);
    int e = !(open & RECT_OPEN_E);
    int s = !(open & RECT_OPEN_S);
    int nw, ne, sw, se;
    int i;
    XGCValues gcValues;
    unsigned long gcMask;
    GC gc;

    /* Dots on even pixels only */
    nw = !(wx & 1) == !(wy & 1);
    ne = !((wx + width - 1) & 1) == !(wy & 1);
    sw = !(wx & 1) == !((wy + height - 1) & 1);
    se = !((wx + width - 1) & 1) == !((wy + height - 1) & 1);

    gcValues.function = GXinvert;
    gcMask = GCFunction;
    gc = Tree_GetGC(tree, gcMask, &gcValues);

    if (w) /* left */
    {
	for (i = !nw; i < height; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x, y + i);
	}
    }
    if (n) /* top */
    {
	for (i = nw ? w * 2 : 1; i < width; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + i, y);
	}
    }
    if (e) /* right */
    {
	for (i = ne ? n * 2 : 1; i < height; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + width - 1, y + i);
	}
    }
    if (s) /* bottom */
    {
	for (i = sw ? w * 2 : 1; i < width - (se && e); i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + i, y + height - 1);
	}
    }
}

/*
 * The following structure is used when drawing a number of dotted XOR
 * rectangles.
 */
struct DotStatePriv
{
    TreeCtrl *tree;
    Drawable drawable;
    GC gc;
    TkRegion rgn;
};

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Setup --
 *
 *	Prepare a drawable for drawing a series of dotted XOR rectangles.
 *
 * Results:
 *	State info is returned to be used by the other TreeDotRect_xxx()
 *	procedures.
 *
 * Side effects:
 *	On Win32 and OSX the device context/graphics port is altered
 *	in preparation for drawing. On X11 a new graphics context is
 *	created.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Setup(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    DotState *p			/* Where to save state info. */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;
    XGCValues gcValues;
    unsigned long mask;
    XRectangle xrect;

    dotState->tree = tree;
    dotState->drawable = drawable;

    gcValues.line_style = LineOnOffDash;
    gcValues.line_width = 1;
    gcValues.dash_offset = 0;
    gcValues.dashes = 1;
    gcValues.function = GXinvert;
    mask = GCLineWidth | GCLineStyle | GCDashList | GCDashOffset | GCFunction;
    dotState->gc = Tk_GetGC(tree->tkwin, mask, &gcValues);

    /* Keep drawing inside the contentbox. */
    dotState->rgn = Tree_GetRegion(tree);
    xrect.x = Tree_ContentLeft(tree);
    xrect.y = Tree_ContentTop(tree);
    xrect.width = Tree_ContentRight(tree) - xrect.x;
    xrect.height = Tree_ContentBottom(tree) - xrect.y;
    TkUnionRectWithRegion(&xrect, dotState->rgn, dotState->rgn);
    TkSetRegion(tree->display, dotState->gc, dotState->rgn);
}

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Draw --
 *
 *	Draw a dotted XOR rectangle.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Draw(
    DotState *p,		/* Info returned by TreeDotRect_Setup(). */
    int x, int y,		/* Left and top coordinates. */
    int width, int height	/* Size of rectangle. */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;

    XDrawRectangle(dotState->tree->display, dotState->drawable, dotState->gc,
	x, y, width - 1, height - 1);
}

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Restore --
 *
 *	Restore the drawing environment.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	On Win32 and OSX the device context/graphics port is restored.
 *	On X11 a new graphics context is freed.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Restore(
    DotState *p			/* Info returned by TreeDotRect_Setup(). */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;

    XSetClipMask(dotState->tree->display, dotState->gc, None);
    Tree_FreeRegion(dotState->tree, dotState->rgn);
    Tk_FreeGC(dotState->tree->display, dotState->gc);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_FillRegion --
 *
 *	Paint a region with the foreground color of a graphics context.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_FillRegion(
    Display *display,		/* Display. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Foreground color. */
    TkRegion rgn		/* Region to paint. */
    )
{
    XRectangle box;

    TkClipBox(rgn, &box);
    TkSetRegion(display, gc, rgn);
    XFillRectangle(display, drawable, gc, box.x, box.y, box.width, box.height);
    XSetClipMask(display, gc, None);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_OffsetRegion --
 *
 *	Offset a region.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_OffsetRegion(
    TkRegion region,		/* Region to modify. */
    int xOffset, int yOffset	/* Horizontal and vertical offsets. */
    )
{
    XOffsetRegion((Region) region, xOffset, yOffset);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_UnionRegion --
 *
 *	Compute the union of 2 regions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_UnionRegion(
    TkRegion rgnA,
    TkRegion rgnB,
    TkRegion rgnOut)
{
    XUnionRegion((Region) rgnA, (Region) rgnB, (Region) rgnOut);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_ScrollWindow --
 *
 *	Wrapper around TkScrollWindow() to fix an apparent bug with the
 *	Mac/OSX versions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is scrolled in a drawable.
 *
 *----------------------------------------------------------------------
 */

int
Tree_ScrollWindow(
    TreeCtrl *tree,		/* Widget info. */
    GC gc,			/* Arg to TkScrollWindow(). */
    int x, int y,		/* Arg to TkScrollWindow(). */
    int width, int height,	/* Arg to TkScrollWindow(). */
    int dx, int dy,		/* Arg to TkScrollWindow(). */
    TkRegion damageRgn		/* Arg to TkScrollWindow(). */
    )
{
    int result = TkScrollWindow(tree->tkwin, gc, x, y, width, height, dx, dy,
	damageRgn);

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_UnsetClipMask --
 *
 *	Wrapper around XSetClipMask(). On Win32 Tk_DrawChars() does
 *	not clear the clipping region.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_UnsetClipMask(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc			/* Graphics context to modify. */
    )
{
    XSetClipMask(tree->display, gc, None);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_DrawBitmapWithGC --
 *
 *	Draw part of a bitmap.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_DrawBitmapWithGC(
    TreeCtrl *tree,		/* Widget info. */
    Pixmap bitmap,		/* Bitmap to draw. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int src_x, int src_y,	/* Left and top of part of bitmap to copy. */
    int width, int height,	/* Width and height of part of bitmap to
				 * copy. */
    int dest_x, int dest_y	/* Left and top coordinates to copy part of
				 * the bitmap to. */
    )
{
    XSetClipOrigin(tree->display, gc, dest_x, dest_y);
    XCopyPlane(tree->display, bitmap, drawable, gc,
	src_x, src_y, (unsigned int) width, (unsigned int) height,
	dest_x, dest_y, 1);
    XSetClipOrigin(tree->display, gc, 0, 0);
}

/*
 * TIP #116 altered Tk_PhotoPutBlock API to add interp arg.
 * We need to remove that for compiling with 8.4.
 */
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 5)
#define TK_PHOTOPUTBLOCK(interp, hdl, blk, x, y, w, h, cr) \
	Tk_PhotoPutBlock(hdl, blk, x, y, w, h, cr)
#define TK_PHOTOPUTZOOMEDBLOCK(interp, hdl, blk, x, y, w, h, \
		zx, zy, sx, sy, cr) \
	Tk_PhotoPutZoomedBlock(hdl, blk, x, y, w, h, \
		zx, zy, sx, sy, cr)
#else
#define TK_PHOTOPUTBLOCK	Tk_PhotoPutBlock
#define TK_PHOTOPUTZOOMEDBLOCK	Tk_PhotoPutZoomedBlock
#endif

/*
 *----------------------------------------------------------------------
 *
 * Tree_XImage2Photo --
 *
 *	Copy pixels from an XImage to a Tk photo image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The given photo image is blanked and all the pixels from the
 *	XImage are put into the photo image.
 *
 *----------------------------------------------------------------------
 */

void
Tree_XImage2Photo(
    Tcl_Interp *interp,		/* Current interpreter. */
    Tk_PhotoHandle photoH,	/* Existing photo image. */
    XImage *ximage,		/* XImage to copy pixels from. */
    unsigned long trans,	/* Pixel value in ximage that should be
				 * considered transparent. */
    int alpha			/* Desired transparency of photo image.*/
    )
{
    Tk_Window tkwin = Tk_MainWindow(interp);
    Display *display = Tk_Display(tkwin);
    Visual *visual = Tk_Visual(tkwin);
    Tk_PhotoImageBlock photoBlock;
    unsigned char *pixelPtr;
    int x, y, w = ximage->width, h = ximage->height;
    int i, ncolors;
    XColor *xcolors;
    unsigned long red_shift, green_shift, blue_shift;
    int separated = 0;

    Tk_PhotoBlank(photoH);

    /* See TkPoscriptImage */

    ncolors = visual->map_entries;
    xcolors = (XColor *) ckalloc(sizeof(XColor) * ncolors);

    if ((visual->class == DirectColor) || (visual->class == TrueColor)) {
	separated = 1;
	red_shift = green_shift = blue_shift = 0;
	/* ximage->red_mask etc are zero */
	while ((0x0001 & (visual->red_mask >> red_shift)) == 0)
	    red_shift++;
	while ((0x0001 & (visual->green_mask >> green_shift)) == 0)
	    green_shift++;
	while ((0x0001 & (visual->blue_mask >> blue_shift)) == 0)
	    blue_shift++;
	for (i = 0; i < ncolors; i++) {
	    xcolors[i].pixel =
		((i << red_shift) & visual->red_mask) |
		((i << green_shift) & visual->green_mask) |
		((i << blue_shift) & visual->blue_mask);
	}
    } else {
	red_shift = green_shift = blue_shift = 0;
	for (i = 0; i < ncolors; i++)
	    xcolors[i].pixel = i;
    }

    XQueryColors(display, Tk_Colormap(tkwin), xcolors, ncolors);

    pixelPtr = (unsigned char *) Tcl_Alloc(ximage->width * ximage->height * 4);
    photoBlock.pixelPtr  = pixelPtr;
    photoBlock.width     = ximage->width;
    photoBlock.height    = ximage->height;
    photoBlock.pitch     = ximage->width * 4;
    photoBlock.pixelSize = 4;
    photoBlock.offset[0] = 0;
    photoBlock.offset[1] = 1;
    photoBlock.offset[2] = 2;
    photoBlock.offset[3] = 3;

    for (y = 0; y < ximage->height; y++) {
	for (x = 0; x < ximage->width; x++) {
	    int r, g, b;
	    unsigned long pixel;

	    pixel = XGetPixel(ximage, x, y);

	    /* Set alpha=0 for transparent pixel in the source XImage */
	    if (trans != 0 && pixel == trans) {
		pixelPtr[y * photoBlock.pitch + x * 4 + 3] = 0;
		continue;
	    }

	    if (separated) {
		r = (pixel & visual->red_mask) >> red_shift;
		g = (pixel & visual->green_mask) >> green_shift;
		b = (pixel & visual->blue_mask) >> blue_shift;
		r = ((double) xcolors[r].red / USHRT_MAX) * 255;
		g = ((double) xcolors[g].green / USHRT_MAX) * 255;
		b = ((double) xcolors[b].blue / USHRT_MAX) * 255;
	    } else {
		r = ((double) xcolors[pixel].red / USHRT_MAX) * 255;
		g = ((double) xcolors[pixel].green / USHRT_MAX) * 255;
		b = ((double) xcolors[pixel].blue / USHRT_MAX) * 255;
	    }
	    pixelPtr[y * photoBlock.pitch + x * 4 + 0] = r;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 1] = g;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 2] = b;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 3] = alpha;
	}
    }

    TK_PHOTOPUTBLOCK(interp, photoH, &photoBlock, 0, 0, w, h,
	    TK_PHOTO_COMPOSITE_SET);

    Tcl_Free((char *) pixelPtr);
    ckfree((char *) xcolors);
}

typedef struct {
    TreeCtrl *tree;
    TreeClip *clip;
    GC gc;
    TkRegion region;
} TreeClipStateGC;

void
TreeClip_ToGC(
    TreeCtrl *tree,		/* Widget info. */
    TreeClip *clip,		/* Clipping area or NULL. */
    GC gc,			/* Graphics context. */
    TreeClipStateGC *state
    )
{
    state->tree = tree;
    state->clip = clip;
    state->gc = gc;
    state->region = None;

    if (clip && clip->type == TREE_CLIP_RECT) {
	state->region = Tree_GetRegion(tree);
	Tree_SetRectRegion(state->region, &clip->tr);
	TkSetRegion(tree->display, gc, state->region);
    }
    if (clip && clip->type == TREE_CLIP_AREA) {
	int x1, y1, x2, y2;
	XRectangle xr;
	if (Tree_AreaBbox(tree, clip->area, &x1, &y1, &x2, &y2) == 0)
	    return;
	xr.x = x1, xr.y = y1, xr.width = x2 - x1, xr.height = y2 - y1;
	state->region = Tree_GetRegion(tree);
	TkUnionRectWithRegion(&xr, state->region, state->region);
	TkSetRegion(tree->display, gc, state->region);
    }
    if (clip && clip->type == TREE_CLIP_REGION) {
	TkSetRegion(tree->display, gc, clip->region);
    }
}

void
TreeClip_FinishGC(
    TreeClipStateGC *state
    )
{
    XSetClipMask(state->tree->display, state->gc, None);
    if (state->region != None)
	Tree_FreeRegion(state->tree, state->region);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_FillRectangle --
 *
 *	Wrapper around XFillRectangle() because the clip region is
 *	ignored on Win32.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Draws onto the specified drawable.
 *
 *----------------------------------------------------------------------
 */

void
Tree_FillRectangle(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeClip *clip,		/* Clipping area or NULL. */
    GC gc,			/* Graphics context. */
    TreeRectangle tr		/* Rectangle to paint. */
    )
{
    TreeClipStateGC clipState;

    TreeClip_ToGC(tree, clip, gc, &clipState);
    XFillRectangle(tree->display, td.drawable, gc, tr.x, tr.y, tr.width, tr.height);
    TreeClip_FinishGC(&clipState);
}

/*** Themes ***/

#ifdef TREECTRL_GTK

#ifdef TREECTRL_DEBUG
/* FIXME: This errors out, even with a small Gtk+ app (no Tcl) */
/*
  gtk_init_check()
    gdk_display_open_default_libgtk_only()
      gdk_display_manager_set_default_display()
        g_object_notify()
          ...
            link_is_locked()          in /usr/lib/libORBit-2.so.0
              link_mutex_is_locked()  in /usr/lib/libORBit-2.so.0
*/
/*#define G_ERRORCHECK_MUTEXES*/
#endif

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <gdk/gdkx.h>

TCL_DECLARE_MUTEX(themeMutex)

/* Per-interp data */
struct TreeThemeData_ {
    TreeItem animButtonItem;
    Tcl_TimerToken animButtonTimer;
    GtkExpanderStyle animButtonStyle;
    int animButtonExpanding;
};

/* Per-application data */
typedef struct {
    gboolean gtk_init;
    int pixbuf_init;
    GtkWidget *gtkWindow;
    GtkWidget *protoLayout;
    GtkWidget *gtkArrow;
    GtkWidget *gtkTreeView;
    GtkWidget *gtkTreeHeader;
    
    /* Cached theme parameters */
    int buttonWidth, buttonHeight;
    int arrowWidth, arrowHeight;
    XColor *textColor[3];
} TreeThemeAppData;

static TreeThemeAppData *appThemeData = NULL;

#define IsGtkUnavailable() ( appThemeData == NULL || appThemeData->gtk_init == 0)

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_DrawHeaderItem --
 *
 *	Draws the background of a single column header.  On Mac OS X
 *	this also draws the sort arrow, if any.
 *
 * Results:
 *	TCL_OK if drawing occurred, TCL_ERROR if the X11 fallback
 *	should be used.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_DrawHeaderItem(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    int state,			/* COLUMN_STATE_xxx flags. */
    int arrow,			/* COLUMN_ARROW_xxx flags. */
    int visIndex,		/* 0-based index in list of visible columns. */
    int x, int y,		/* Bounds of the header. */
    int width, int height	/* Bounds of the header. */
    )
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state_type = GTK_STATE_NORMAL;
    GtkShadowType shadow_type = GTK_SHADOW_OUT;
    GdkRectangle area = {0, 0, width, height}; /* clip */
    GdkPixmap *gdkPixmap;
    GdkPixbuf *pixbuf;
    
    if (IsGtkUnavailable() || appThemeData->gtkTreeHeader == NULL)
	return TCL_ERROR;

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkTreeHeader;
    style = gtk_widget_get_style(widget);
    
    switch (state) {
	case COLUMN_STATE_ACTIVE:
	    state_type = GTK_STATE_PRELIGHT;
	    break;
	case COLUMN_STATE_PRESSED:
	    state_type = GTK_STATE_ACTIVE;
	    shadow_type = GTK_SHADOW_IN;
	    break;
	case COLUMN_STATE_NORMAL: 
	    break;
    }

    /* Allocate GdkPixmap to draw background in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	goto ret_error;
    }
    
    /* Paint the background */
    gtk_paint_box (style, gdkPixmap, state_type, shadow_type, &area, widget,
	"button", 0, 0, width, height);

    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(NULL, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	goto ret_error;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_NONE, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_OK;
ret_error:
    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_GetHeaderContentMargins --
 *
 *	Returns the padding inside the column header borders where
 *	text etc may be displayed.
 *
 * Results:
 *	TCL_OK if 'bounds' was set, TCL_ERROR otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_GetHeaderContentMargins(
    TreeCtrl *tree,		/* Widget info. */
    int state,			/* COLUMN_STATE_xxx flags. */
    int arrow,			/* COLUMN_ARROW_xxx flags. */
    int bounds[4]		/* Returned left-top-right-bottom padding. */
    )
{
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_DrawHeaderArrow --
 *
 *	Draws the sort arrow in a column header.
 *
 * Results:
 *	TCL_OK if drawing occurred, TCL_ERROR if the X11 fallback
 *	should be used.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_DrawHeaderArrow(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    int state,			/* COLUMN_STATE_xxx flags. */
    int up,			/* TRUE if up arrow, FALSE otherwise. */
    int x, int y,		/* Bounds of arrow.  Width and */
    int width, int height	/* height are the same as that returned */
				/* by TreeTheme_GetArrowSize(). */ 
    )
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state_type = GTK_STATE_NORMAL;
    GdkRectangle area = {0, 0, width, height}; /* clip */
    const gchar *detail = "arrow";
    GtkShadowType shadow_type;
    GtkArrowType effective_arrow_type = up ? GTK_ARROW_DOWN : GTK_ARROW_UP; /* INVERTED!!! */
    GdkPixmap *gdkPixmap;
    GdkRectangle clipped;
    GdkPixbuf *pixbuf;

    if (IsGtkUnavailable() || appThemeData->gtkArrow == NULL)
	return TCL_ERROR;

    clipped.x = x, clipped.y = y, clipped.width = width, clipped.height = height;
    if (clipped.x < 0)
	clipped.width += clipped.x, clipped.x = 0;
    if (clipped.y < 0)
	clipped.height += clipped.y, clipped.y = 0;
    if (clipped.width < 1 || clipped.height < 1)
	return TCL_OK;

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkArrow;
    style = gtk_widget_get_style(widget);
    shadow_type = GTK_ARROW(widget)->shadow_type;
   
    switch (state) {
	case COLUMN_STATE_ACTIVE:
	    state_type = GTK_STATE_PRELIGHT;
	    break;
	case COLUMN_STATE_PRESSED:
	    state_type = GTK_STATE_ACTIVE;
	    break;
	case COLUMN_STATE_NORMAL: 
	    break;
    }

    if (appThemeData->gtkTreeView != NULL) {
	gboolean alternative = FALSE;
	GtkSettings *settings = gtk_widget_get_settings(appThemeData->gtkTreeView);
	g_object_get(settings, "gtk-alternative-sort-arrows", &alternative, NULL);
	if (alternative)
	    effective_arrow_type = up ? GTK_ARROW_UP : GTK_ARROW_DOWN;
    }

    /* This gives warning "widget class `GtkArrow' has no property named `shadow-type'" */
/*
    gtk_widget_style_get(widget, "shadow-type", &shadow_type, NULL);
*/

    /* Copy background from Tk Pixmap -> GdkPixbuf */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, width, height);
    if (pixbuf == NULL)
	goto ret_error;
    pixbuf = gdk_pixbuf_xlib_get_from_drawable(pixbuf, drawable,
	Tk_Colormap(tree->tkwin), Tk_Visual(tree->tkwin), clipped.x, clipped.y,
	x < 0 ? -x : 0, y < 0 ? -y : 0, clipped.width, clipped.height);
    if (pixbuf == NULL)
	goto ret_error;
    
    /* Allocate GdkPixmap to draw button in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	gdk_pixbuf_unref(pixbuf);
	goto ret_error;
    }
    
    /* Copy GdkPixbuf containing background to GdkPixmap */
    gdk_pixbuf_render_to_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height, GDK_RGB_DITHER_NONE, 0, 0);

    /* Draw the button */
    gtk_paint_arrow(style, gdkPixmap, state_type, shadow_type, &area, widget,
	detail, effective_arrow_type, TRUE, 0, 0, width, height);
    
    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	goto ret_error;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_MAX, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_OK;
ret_error:
    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_DrawButton --
 *
 *	Draws a single expand/collapse item button.
 *
 * Results:
 *	TCL_OK if drawing occurred, TCL_ERROR if the X11 fallback
 *	should be used.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_DrawButton(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    TreeItem item,		/* Needed for animating. */
    int state,			/* STATE_xxx | BUTTON_STATE_xxx flags. */
    int x, int y,		/* Bounds of the button.  Width and height */
    int width, int height	/* are the same as that returned by */
				/* TreeTheme_GetButtonSize(). */
    )
{
    int open = state & STATE_OPEN;
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state_type = GTK_STATE_NORMAL;
    GdkRectangle area = {0, 0, width, height}; /* clip */
    const gchar *detail = "treeview";
    GtkExpanderStyle expander_style = open ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED;
    GdkPixmap *gdkPixmap;
    GdkRectangle clipped;
    GdkPixbuf *pixbuf;

    if (IsGtkUnavailable() || appThemeData->gtkTreeView == NULL)
	return TCL_ERROR;

    clipped.x = x, clipped.y = y, clipped.width = width, clipped.height = height;
    if (clipped.x < 0)
	clipped.width += clipped.x, clipped.x = 0;
    if (clipped.y < 0)
	clipped.height += clipped.y, clipped.y = 0;
    if (clipped.width < 1 || clipped.height < 1)
	return TCL_OK;

    if (state & BUTTON_STATE_ACTIVE)
	state_type = GTK_STATE_PRELIGHT;
    else if (state & BUTTON_STATE_PRESSED)
	state_type = GTK_STATE_ACTIVE;

    if (item == tree->themeData->animButtonItem) {
	expander_style = tree->themeData->animButtonStyle;
    }

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkTreeView;
    style = gtk_widget_get_style(widget);

    /* Copy background from Tk Pixmap -> GdkPixbuf */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, width, height);
    if (pixbuf == NULL)
	goto ret_error;
    pixbuf = gdk_pixbuf_xlib_get_from_drawable(pixbuf, drawable,
	Tk_Colormap(tree->tkwin), Tk_Visual(tree->tkwin), clipped.x, clipped.y,
	x < 0 ? -x : 0, y < 0 ? -y : 0, clipped.width, clipped.height);
    if (pixbuf == NULL)
	goto ret_error;
    
    /* Allocate GdkPixmap to draw button in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	gdk_pixbuf_unref(pixbuf);
	goto ret_error;
    }
    
    /* Copy GdkPixbuf containing background to GdkPixmap */
    gdk_pixbuf_render_to_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height, GDK_RGB_DITHER_NONE, 0, 0);

    /* Draw the button */
    gtk_paint_expander(style, gdkPixmap, state_type, &area, widget, detail,
	width / 2, height / 2, expander_style);
    
    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	goto ret_error;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_MAX, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_OK;
ret_error:
    gdk_threads_leave(); /* +++ release global mutex +++ */
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_GetButtonSize --
 *
 *	Returns the width and height of an expand/collapse item button.
 *
 * Results:
 *	TCL_OK if *widthPtr and *heightPtr were set, TCL_ERROR
 *	if themed buttons can't be drawn.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_GetButtonSize(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Needed on MS Windows. */
    int open,			/* TRUE if expanded button. */
    int *widthPtr,		/* Returned width of button. */
    int *heightPtr		/* Returned height of button. */
    )
{
    GtkWidget *widget;
    const gchar *property_name = "expander-size";
    gint expander_size;

    if (IsGtkUnavailable() || appThemeData->gtkTreeView == NULL)
	return TCL_ERROR;

    /* Use the cached value if available */
    if (appThemeData->buttonWidth > 0) {
    	(*widthPtr) = appThemeData->buttonWidth;
    	(*heightPtr) = appThemeData->buttonHeight;
    	return TCL_OK;
    }

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkTreeView;

    gtk_widget_style_get(widget, property_name, &expander_size, NULL);

    gdk_threads_leave(); /* +++ release global mutex +++ */

    appThemeData->buttonWidth = appThemeData->buttonHeight = expander_size;
    (*widthPtr) = (*heightPtr) = expander_size;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_GetArrowSize --
 *
 *	Returns the width and height of a column header sort arrow.
 *
 * Results:
 *	TCL_OK if *widthPtr and *heightPtr were set, TCL_ERROR
 *	if themed sort arrows can't be drawn.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_GetArrowSize(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Needed on MS Windows. */
    int up,			/* TRUE if up arrow. */
    int *widthPtr,		/* Returned width of arrow. */
    int *heightPtr		/* Returned height of arrow. */
    )
{
    GtkWidget *widget;
    GtkRequisition requisition;
    gfloat arrow_scaling = 1.0f;
    GtkMisc *misc;
    gint width, height, extent;
     
    if (IsGtkUnavailable() || appThemeData->gtkArrow == NULL)
	return TCL_ERROR;

    /* Use the cached value if available */
    if (appThemeData->arrowWidth > 0) {
    	(*widthPtr) = appThemeData->arrowWidth;
    	(*heightPtr) = appThemeData->arrowHeight;
    	return TCL_OK;
    }

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkArrow;
    misc = GTK_MISC(widget);
    
    gtk_widget_size_request(widget, &requisition);
    gtk_widget_style_get(widget, "arrow-scaling", &arrow_scaling, NULL);
    width = requisition.width - misc->xpad * 2;
    height = requisition.height - misc->ypad * 2;
    extent = MIN(width, height) * arrow_scaling;

    gdk_threads_leave(); /* +++ release global mutex +++ */

    appThemeData->arrowWidth = appThemeData->arrowHeight = extent;
    (*widthPtr) = (*heightPtr) = extent;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_SetBorders --
 *
 *	Sets the TreeCtrl.inset pad values according to the needs of
 *	the system theme.
 *
 * Results:
 *	TCL_OK if the inset was set, TCL_ERROR if the -highlightthickness
 *	and -borderwidth values should be used.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_SetBorders(
    TreeCtrl *tree		/* Widget info. */
    )
{
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_DrawBorders --
 *
 *	Draws themed borders around the edges of the treectrl.
 *
 * Results:
 *	TCL_OK if drawing occurred, TCL_ERROR if the Tk focus rectangle
 *	and 3D border should be drawn.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_DrawBorders(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable		/* Where to draw. */
    )
{
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_GetColumnTextColor --
 *
 *	Returns the text fill color to display a column title with.
 *
 * Results:
 *	TCL_OK if the *colorPtrPtr was set, TCL_ERROR if a non-theme
 *	color should be used.
 *
 * Side effects:
 *	May allocate a new XColor.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_GetColumnTextColor(
    TreeCtrl *tree,		/* Widget info. */
    int columnState,		/* COLUMN_STATE_xxx flags. */
    XColor **colorPtrPtr	/* Returned text color. */
    )
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state_type = GTK_STATE_NORMAL;
    int colorIndex = 0;
    GdkColor *gdkColor;
    XColor pref;

    if (IsGtkUnavailable() || appThemeData->gtkTreeHeader == NULL)
	return TCL_ERROR;

     switch (columnState) {
	case COLUMN_STATE_ACTIVE:
	    state_type = GTK_STATE_PRELIGHT;
	    colorIndex = 1;
	    break;
	case COLUMN_STATE_PRESSED:
	    state_type = GTK_STATE_ACTIVE;
	    colorIndex = 2;
	    break;
	case COLUMN_STATE_NORMAL:
	    break;
    }
    
    /* Use the cached value if available */
    if (appThemeData->textColor[colorIndex] != NULL) {
    	(*colorPtrPtr) = appThemeData->textColor[colorIndex];
    	return TCL_OK;
    }
    
    gdk_threads_enter(); /* +++ grab global mutex +++ */

    widget = appThemeData->gtkTreeHeader;
    style = gtk_widget_get_style(widget);

    gdkColor = &style->fg[state_type];

    pref.red = gdkColor->red;
    pref.green = gdkColor->green;
    pref.blue = gdkColor->blue;
    appThemeData->textColor[colorIndex] = Tk_GetColorByValue(tree->tkwin, &pref);
    (*colorPtrPtr) = appThemeData->textColor[colorIndex];

    gdk_threads_leave(); /* +++ release global mutex +++ */

    return TCL_OK;
}

#define ANIM_BUTTON_INTERVAL 50 /* same as gtk_treeview */

static void
AnimButtonTimerProc(
    ClientData clientData
    )
{
    TreeCtrl *tree = clientData;
    int finished = 0;

    if (tree->themeData->animButtonExpanding) {
	if (tree->themeData->animButtonStyle == GTK_EXPANDER_SEMI_COLLAPSED)
	    tree->themeData->animButtonStyle = GTK_EXPANDER_SEMI_EXPANDED;
	else
	    finished = 1;
    } else {
	if (tree->themeData->animButtonStyle == GTK_EXPANDER_SEMI_EXPANDED)
	    tree->themeData->animButtonStyle = GTK_EXPANDER_SEMI_COLLAPSED;
	else
	    finished = 1;
    }

    Tree_InvalidateItemDInfo(tree, tree->columnTree,
	tree->themeData->animButtonItem, NULL);

    if (finished) {
	tree->themeData->animButtonTimer = NULL;
	tree->themeData->animButtonItem = NULL;
    } else {
	tree->themeData->animButtonTimer = Tcl_CreateTimerHandler(
	    ANIM_BUTTON_INTERVAL, AnimButtonTimerProc, tree);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_AnimateButtonStart --
 *
 *	Starts an expand/collapse item button animating from open to
 *	closed or vice versa.
 *
 * Results:
 *	TCL_OK.
 *
 * Side effects:
 *	May create a new Tcl_TimerToken.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_AnimateButtonStart(
    TreeCtrl *tree,		/* Widget info. */
    TreeItem item		/* The item whose button should animate. */
    )
{
    int open = (TreeItem_GetState(tree, item) & STATE_OPEN) != 0;
    int animate;

    /* gtk_treeview toggles right away, not when the animation finishes. */
    TreeItem_OpenClose(tree, item, -1);
#ifdef SELECTION_VISIBLE
    Tree_DeselectHidden(tree);
#endif

    if (IsGtkUnavailable() || appThemeData->gtkTreeView == NULL) {
	return TCL_OK;
    }

    gdk_threads_enter(); /* +++ grab global mutex +++ */

    g_object_get(gtk_widget_get_settings(GTK_WIDGET(appThemeData->gtkTreeView)),
	"gtk-enable-animations", &animate, NULL);

    gdk_threads_leave(); /* +++ release global mutex +++ */

    if (!animate) {
	return TCL_OK;
    }

    if (tree->themeData->animButtonTimer != NULL)
	Tcl_DeleteTimerHandler(tree->themeData->animButtonTimer);

    tree->themeData->animButtonItem = item;
    tree->themeData->animButtonTimer = Tcl_CreateTimerHandler(
	ANIM_BUTTON_INTERVAL, AnimButtonTimerProc, tree);
    tree->themeData->animButtonExpanding = !open;
    if (open)
	tree->themeData->animButtonStyle = GTK_EXPANDER_SEMI_EXPANDED;
    else
	tree->themeData->animButtonStyle = GTK_EXPANDER_SEMI_COLLAPSED;
    
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_ItemDeleted --
 *
 *	Cancels any item-button animation in progress.
 *
 * Results:
 *	TCL_OK.
 *
 * Side effects:
 *	May delete a TCL_TimerToken.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_ItemDeleted(
    TreeCtrl *tree,		/* Widget info. */
    TreeItem item		/* Item being deleted. */
    )
{
    if (item != tree->themeData->animButtonItem)
	return TCL_OK;
    if (tree->themeData->animButtonTimer != NULL) {
	Tcl_DeleteTimerHandler(tree->themeData->animButtonTimer);
	tree->themeData->animButtonTimer = NULL;
	tree->themeData->animButtonItem = NULL;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_Relayout --
 *
 *	This gets called when certain config options change and when
 *	the size of the widget changes.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
TreeTheme_Relayout(
    TreeCtrl *tree		/* Widget info. */
    )
{
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_IsDesktopComposited --
 *
 *	Determine if the OS windowing system is composited AKA
 *	double-buffered.
 *
 * Results:
 *	FALSE FALSE FALSE.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_IsDesktopComposited(
    TreeCtrl *tree		/* Widget info. */
    )
{
    return FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_ThemeChanged --
 *
 *	Called after the system theme changes.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
TreeTheme_ThemeChanged(
    TreeCtrl *tree		/* Widget info. */
    )
{
}

#if 0
/* _GTK_READ_RCFILES ClientMessage sent to X toplevel windows with WM_STATE property */
/* gtk_rc_reparse_all_for_settings(gtk_widget_get_settings(widget), FALSE); */

static int ClientMessageHandler(Tk_Window tkwin, XEvent *eventPtr) {
    dbwin("ClientMessageHandler type=%d\n",eventPtr->type);
    fprintf(stderr,"ClientMessageHandler type=%d\n",eventPtr->type);
    return 0;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_Init --
 *
 *	Performs theme-related initialization when a treectrl is
 *	created.
 *
 * Results:
 *	TCL_OK or TCL_ERROR, but result is ignored.
 *
 * Side effects:
 *	Depends on the platform.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_Init(
    TreeCtrl *tree		/* Widget info. */
    )
{
    tree->themeData = (TreeThemeData) ckalloc(sizeof(struct TreeThemeData_));
    tree->themeData->animButtonTimer = NULL;

    Tcl_MutexLock(&themeMutex);

    if (IsGtkUnavailable() == 0 && appThemeData->pixbuf_init == 0) {
	Tk_MakeWindowExist(tree->tkwin);
	
	gdk_threads_enter(); /* +++ grab global mutex +++ */

	gdk_pixbuf_xlib_init(Tk_Display(tree->tkwin), 0);
	xlib_rgb_init(Tk_Display(tree->tkwin), Tk_Screen(tree->tkwin));

	gdk_threads_leave(); /* +++ release global mutex +++ */
	
	appThemeData->pixbuf_init = 1;
    }
    
    Tcl_MutexUnlock(&themeMutex);
#if 0
    Tk_CreateClientMessageHandler(ClientMessageHandler);
#endif
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_Free --
 *
 *	Performs theme-related cleanup a when a treectrl is destroyed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on the platform.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_Free(
    TreeCtrl *tree		/* Widget info. */
    )
{
    if (tree->themeData != NULL) {
	ckfree((char *) tree->themeData);
	tree->themeData = NULL;
    }

    return TCL_OK;
}

#if 0
static void StyleSetCallback(
    GtkWidget *widget,
    GtkStyle *previous_style,
    gpointer user_data)
{
    Tcl_Interp *interp = user_data;
    dbwin("StyleSetCallback\n");
/*    Tree_TheWorldHasChanged(interp);*/
}

static int
ThemeChangedObjCmd(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    if (IsGtkUnavailable() == 0) {
       int i;
       const gchar *theme_name;
       GtkSettings *settings = gtk_widget_get_settings(appThemeData->gtkWindow);
       GtkRcContext *context = settings ? settings->rc_context : NULL;
       if (context == NULL) return TCL_OK;
       g_object_get(settings, "gtk-theme-name", &theme_name, NULL);
       dbwin("context->theme_name %s\n", theme_name);
       dbwin("gtk_rc_get_theme_dir %s\n", gtk_rc_get_theme_dir());
       for (i = 0; gtk_rc_get_default_files()[i] != NULL; i++) {
       	    dbwin("gtk_rc_parse %s\n", gtk_rc_get_default_files()[i]);
//            gtk_rc_parse(gtk_rc_get_default_files()[i]);
        }
        gtk_rc_reparse_all_for_settings(settings, TRUE); /* calls StyleSetCallback! */
    }
    return TCL_OK;
}
#endif

static int (*TkXErrorHandler)(Display *displayPtr, XErrorEvent *errorPtr);
static int TreeCtrlErrorHandler(Display *displayPtr, XErrorEvent *errorPtr)
{
    return TkXErrorHandler(displayPtr, errorPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * TreeTheme_InitInterp --
 *
 *	Performs theme-related initialization when the TkTreeCtrl
 *	package is loaded into an interpreter.
 *
 * Results:
 *	TCL_OK or TCL_ERROR, but result is ignored.
 *
 * Side effects:
 *	Depends on the platform.
 *
 *----------------------------------------------------------------------
 */

int
TreeTheme_InitInterp(
    Tcl_Interp *interp		/* Interp that loaded TkTreeCtrl pkg. */
    )
{
    Tcl_MutexLock(&themeMutex);
    
    if (appThemeData == NULL) {
	int argc = 1;
	char **argv = g_new0(char*, 2);
	argv[0] = (char *) Tcl_GetNameOfExecutable();
	GtkTreeViewColumn *column;

	if (!g_thread_supported()) {
	    /* Initialized Glib threads.  Must be done before gdk_threads_init(). */
	    g_thread_init(NULL);

	    /* Initialize Gdk threads.  Should be done before gtk_init(). */
	    /* FIXME: it isn't ok to call this multiple times, it creates mutexes each time, could
	     * be an embedded interp in a Gtk+ application which already called this. 
	     * Assume it hasn't been done if g_thread_init() wasn't called. */
	    gdk_threads_init();
	}


	appThemeData = (TreeThemeAppData*) ckalloc(sizeof(TreeThemeAppData));
	memset(appThemeData, '\0', sizeof(TreeThemeAppData));

	gdk_threads_enter(); /* +++ grab global mutex +++ */
	
	/* Gtk+ installs an X error handler which terminates the application.
	 * Install our own error handler so we can get the Tk error handler
	 * before Gtk+ installs its own.  After initializing Gtk+ the Tk
	 * error handler is restored. */
	TkXErrorHandler = XSetErrorHandler(TreeCtrlErrorHandler);
	
#if 0
	/* This must be called before gtk_init(). */
	/* FIXME: this might already have been called (from tile-gtk, or if this is an
	 * an embedded interp in a Gtk+ application which already called gtk_init. */
	gtk_disable_setlocale();
#endif
	appThemeData->gtk_init = gtk_init_check(&argc, &argv);
	g_free(argv);
	if (!appThemeData->gtk_init) {
	    XSetErrorHandler(TkXErrorHandler);
	    gdk_threads_leave(); /* +++ release global mutex +++ */
	    Tcl_MutexUnlock(&themeMutex);
	    return TCL_ERROR;
	}

	appThemeData->gtkWindow = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_realize(appThemeData->gtkWindow);

	appThemeData->protoLayout = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(appThemeData->gtkWindow),
	    appThemeData->protoLayout);

	appThemeData->gtkTreeView = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(appThemeData->protoLayout),
	    appThemeData->gtkTreeView);
	gtk_widget_realize(appThemeData->gtkTreeView);
#if 0
	g_signal_connect(G_OBJECT(appThemeData->gtkTreeView), "style-set",
	    G_CALLBACK(StyleSetCallback), interp); /* FIXME: all interps, which thread */
#endif
	/* GTK_SHADOW_IN is default for GtkTreeView */
	appThemeData->gtkArrow = gtk_arrow_new(GTK_ARROW_NONE, GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(appThemeData->protoLayout),
	    appThemeData->gtkArrow);
	gtk_widget_realize(appThemeData->gtkArrow);

	/* Create *three* columns, and use the middle column when drawing
	 * headers. */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
	appThemeData->gtkTreeHeader = column->button;
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
	
	XSetErrorHandler(TkXErrorHandler);
	
	gdk_threads_leave(); /* +++ release global mutex +++ */
    }
    
    Tcl_MutexUnlock(&themeMutex);
#if 0
    Tcl_CreateObjCommand(interp, "gtk-theme-changed", ThemeChangedObjCmd, NULL, NULL);
#endif
    return TCL_OK;
}

#endif /* TREECTRL_GTK */

#ifndef TREECTRL_GTK
int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int visIndex, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    return TCL_ERROR;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int state, int up, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, TreeItem item, int open, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
}


int TreeTheme_SetBorders(TreeCtrl *tree)
{
    return TCL_ERROR;
}

int
TreeTheme_DrawBorders(
    TreeCtrl *tree,
    Drawable drawable
    )
{
    return TCL_ERROR;
}

int TreeTheme_GetColumnTextColor(
    TreeCtrl *tree,
    int columnState,
    XColor **colorPtrPtr)
{
    return TCL_ERROR;
}

void
TreeTheme_Relayout(
    TreeCtrl *tree
    )
{
}

int
TreeTheme_IsDesktopComposited(
    TreeCtrl *tree
    )
{
    return FALSE;
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
}

int TreeTheme_Init(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    return TCL_OK;
}

#endif /* not TREECTRL_GTK */

int 
TreeThemeCmd(
    TreeCtrl *tree,		/* Widget info. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    Tcl_Interp *interp = tree->interp;
    static CONST char *commandName[] = {
	"platform", (char *) NULL
    };
    enum {
	COMMAND_PLATFORM
    };
    int index;

    if (objc < 3) {
	Tcl_WrongNumArgs(interp, 2, objv, "command ?arg arg ...?");
	return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[2], commandName, "command", 0,
	    &index) != TCL_OK) {
	return TCL_ERROR;
    }

    switch (index) {
	/* T theme platform */
	case COMMAND_PLATFORM: {
	    char *platform = "X11";
#ifdef TREECTRL_GTK
	    if (IsGtkUnavailable() == 0)
		platform = "gtk";
#endif
	    Tcl_SetObjResult(interp, Tcl_NewStringObj(platform, -1));
	    break;
	}
    }

    return TCL_OK;
}

/*** Gradients ***/

/*
 *----------------------------------------------------------------------
 *
 * Tree_HasNativeGradients --
 *
 *	Determine if this platform supports gradients natively.
 *
 * Results:
 *	0.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tree_HasNativeGradients(
    TreeCtrl *tree)
{
    return 0; /* TODO: cairo */
}

/*
 *----------------------------------------------------------------------
 *
 * TreeGradient_FillRect --
 *
 *	Paint a rectangle with a gradient.
 *
 * Results:
 *	If the gradient has <2 stops then nothing is drawn.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

void
TreeGradient_FillRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeClip *clip,		/* Clipping area or NULL. */
    TreeGradient gradient,	/* Gradient token. */
    TreeRectangle trBrush,	/* Brush bounds. */
    TreeRectangle tr		/* Rectangle to paint. */
    )
{
    TreeGradient_FillRectX11(tree, td, clip, gradient, trBrush, tr);

    /* FIXME: Can use 'cairo' on Unix, but need to add it to configure + Make */
}

void
TreeGradient_FillRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeGradient gradient,	/* Gradient token. */
    TreeRectangle trBrush,	/* Brush bounds. */
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    TreeGradient_FillRoundRectX11(tree, td, NULL, gradient, trBrush, tr, rx, ry, open);

    /* FIXME: Can use 'cairo' on Unix, but need to add it to configure + Make */
}

void
Tree_DrawRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    XColor *xcolor,		/* Color. */
    TreeRectangle tr,		/* Where to draw. */
    int outlineWidth,
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    /* FIXME: MacOSX + Cocoa, Unix + cairo */
    GC gc = Tk_GCForColor(xcolor, Tk_WindowId(tree->tkwin));
    Tree_DrawRoundRectX11(tree, td, gc, tr, outlineWidth, rx, ry, open);
}

void
Tree_FillRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    XColor *xcolor,		/* Color. */
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    /* FIXME: MacOSX + Cocoa, Unix + cairo */
    GC gc = Tk_GCForColor(xcolor, Tk_WindowId(tree->tkwin));
    Tree_FillRoundRectX11(tree, td, gc, tr, rx, ry, open);
}

int
TreeDraw_InitInterp(
    Tcl_Interp *interp
    )
{
    return TCL_OK;
}

