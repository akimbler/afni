#ifndef _MCW_DISPLAY_HEADER_
#define _MCW_DISPLAY_HEADER_

/*****************************************************************************
  This software is copyrighted and owned by the Medical College of Wisconsin.
  See the file README.Copyright for details.
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>

#include "mrilib.h"

/*** Macros ***/

#ifndef MAX
#   define MAX(a,b) (((a)<(b)) ? (b) : (a))
#   define MIN(a,b) (((a)>(b)) ? (b) : (a))
#endif

#ifndef myXtFree
# define myXtFree(xp) (XtFree((char *)(xp)) , (xp)=NULL)
#endif

#ifndef myXtNew
# define myXtNew(type) ((type *) XtCalloc(1,(unsigned) sizeof(type)))
#endif

/* these macros are to produce RGB intensities (unsigned shorts) */

#define CLIP_INTEN(i)    (((i)<256) ? (256) : ((i)>65280) ? (65280) : (i))
#define BYTE_TO_INTEN(b) (CLIP_INTEN((b)<<8))
#define INTEN_TO_BYTE(i) ((i)>>8)

#define BRIGHTNESS(r,g,b)   (0.299*(r)+0.587*(g)+0.114*(b))
#define XCOL_BRIGHTNESS(xc) BRIGHTNESS((xc).red,(xc).green,(xc).blue)

#define XCOL_REDNESS(xc)    (0.299*(xc).red   - MAX(0.587*(xc).green,0.114*(xc).blue ))
#define XCOL_GREENNESS(xc)  (0.587*(xc).green - MAX(0.299*(xc).red  ,0.114*(xc).blue ))
#define XCOL_BLUENESS(xc)   (0.114*(xc).blue  - MAX(0.299*(xc).red  ,0.587*(xc).green))

/* given x in [0..wx-1], map proportionally to [0..wn-1] */

#define MAP_XY(x,wx,wn) (((wn)*(x))/(wx))

/* 07 Aug 1998:
   Macro to produce TrueColor pixel values from
   an RGB triple, by appropriately shifting and masking.
   This can only be used if dc->visual_class == TrueColor! */

#define RGB_TO_TCINT(dc,r,g,b)                                       \
 ( ((((dc)->visual_redshift  <0)                                     \
      ? ((r)<<(-(dc)->visual_redshift)  )                            \
      : ((r)>>  (dc)->visual_redshift)  ) & (dc)->visual_redmask  )  \
  |                                                                  \
   ((((dc)->visual_greenshift<0)                                     \
      ? ((g)<<(-(dc)->visual_greenshift))                            \
      : ((g)>>  (dc)->visual_greenshift)) & (dc)->visual_greenmask)  \
  |                                                                  \
   ((((dc)->visual_blueshift <0)                                     \
      ? ((b)<<(-(dc)->visual_blueshift) )                            \
      : ((b)>>  (dc)->visual_blueshift) ) & (dc)->visual_bluemask ) )

/*--- 11 Feb 1999: stuff for mapping colors to pixels ------------------*/

typedef struct {
   int class ;    /* type of colormap: PseudoColor and TrueColor are OK */
   int depth ;

   int ncolors , nblack,nwhite ;  /* This stuff for PseudoColor */
   byte * rr , * gg , * bb ;

   unsigned long rrmask , ggmask , bbmask ;   /* This stuff for TrueColor */
   int           rrshift, ggshift, bbshift;
   Pixel         whpix ;
} DC_colordef ;

#define FREE_DC_colordef(cd)                                     \
  do{ if( (cd) != NULL ){                                        \
         if( (cd)->rr != NULL ){                                 \
            free((cd)->rr) ; free((cd)->gg) ; free((cd)->bb) ; } \
         free((cd)) ; (cd) = NULL ; } } while(0)

/***---------------------------- typedefs ----------------------------***/

#define MAX_COLORS 256

/** Dec 1997: split overlay stuff into a separate struct **/

typedef struct {
   int    ncol_ov ;              /* number of overlay colors defined */
   XColor xcol_ov[MAX_COLORS] ;  /* definitions of overlay colors */
   Pixel  pix_ov[MAX_COLORS]  ;  /* Pixels for overlay */
   char * name_ov[MAX_COLORS] ;  /* names of overlay colors */
   char * label_ov[MAX_COLORS] ; /* labels for overlay colors */

   Pixel  pixov_brightest,pixov_darkest,pixov_reddest,pixov_greenest,pixov_bluest;
   int    ov_brightest,   ov_darkest,   ov_reddest,   ov_greenest,   ov_bluest;

   float  bright_ov[MAX_COLORS] ; /* brightness of overlay colors [20 Dec 1999] */
} MCW_DCOV ;

#define DCOV_REDBYTE(dc,i)   INTEN_TO_BYTE((dc)->ovc->xcol_ov[i].red)
#define DCOV_GREENBYTE(dc,i) INTEN_TO_BYTE((dc)->ovc->xcol_ov[i].green)
#define DCOV_BLUEBYTE(dc,i)  INTEN_TO_BYTE((dc)->ovc->xcol_ov[i].blue)
#define DC_GRAYBYTE(dc,i)    INTEN_TO_BYTE((dc)->xgry_im[i].red)

#define DCOV_BRIGHTNESS(dc,i) ((dc)->ovc->bright_ov[i])

typedef struct {
      XtAppContext appcontext ;    /* X and Xt stuff */
      Display *    display ;
      Screen *     screen ;
      int          screen_num ;
      Visual *     visual ;
      Colormap     colormap , default_colormap ;
      GC           myGC , origGC ;
      int          planes ;
      int          depth ;

      VisualID      visual_id ;     /* 07 Aug 1998: added visual_* stuff */
      XVisualInfo * visual_info ;
      unsigned long visual_redmask  , visual_greenmask  , visual_bluemask ;
      int           visual_redshift , visual_greenshift , visual_blueshift ;
      int           visual_class ;

      int          width , height ;       /* of the screen */

      int          ncol_im ;              /* # colors we use */
      double       gamma , gamma_init ;   /* gamma factor */
      int          use_xcol_im ;          /* color in use? */

      XColor       xgry_im[MAX_COLORS] ,  /* for images */
                   xcol_im[MAX_COLORS]  ;

      Pixel        pix_im[MAX_COLORS] ;
      int          pix_im_ready ;         /* 22 Aug 1998 */
      int          byper , bypad ;        /* 23 Aug 1998 */

      MCW_DCOV *   ovc ;                  /* Dec 1997 */

      int          xint_im[MAX_COLORS] ;  /* intensity levels for xgry_im */

      XFontStruct * myFontStruct ;

      Widget       parent_widget ;

      XtPointer    parent , aux ;

      DC_colordef * cdef ;   /* 11 Feb 1999 */
} MCW_DC ;

/* fonts to try if the defaults fail */

static char * tfont_hopefuls[] = {
                 "-adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1" ,
                 "-misc-fixed-medium-r-normal--13-100-100-100-c-70-iso8859-1" ,
                 "lucidasanstypewriter-10" ,
                 "7x14" , "6x13" , "fixed" ,
              NULL } ;

/*** Macro for text widths ***/

#define DC_text_width(dc,str) XTextWidth((dc)->myFontStruct,(str),strlen((str)))

/*** prototypes ***/

extern void DC_yokify( Widget , MCW_DC * ) ; /* 14 Sep 1998 */

extern MCW_DC * MCW_new_DC( Widget, int, int, char * c[], char * l[], double, int ) ;

extern void DC_init_im_gry( MCW_DC * ) ;
extern void DC_init_im_col( MCW_DC * ) ;
extern void DC_init_ov_col( MCW_DC * ) ;

#if 0
extern Pixel RGB_byte_to_color( MCW_DC *, int,int,int ) ;
extern Pixel Name_to_color( MCW_DC * , char * ) ;
#endif

extern int DC_add_overlay_color( MCW_DC * , char * , char * ) ;
extern int DC_find_overlay_color( MCW_DC * , char * ) ;

extern void load_tmp_colors( int , XColor c[] ) ;

extern void DC_palette_rotate( MCW_DC * , int ) ;
extern void DC_palette_swap( MCW_DC * ) ;

extern void DC_palette_bright( MCW_DC * , int ) ;
extern void DC_palette_squeeze( MCW_DC * , int ) ;

extern void DC_palette_restore( MCW_DC * , double ) ;

extern void DC_gray_change( MCW_DC * , int ) ;
extern void DC_color_bright( MCW_DC * , int ) ;

extern void DC_gray_contrast( MCW_DC * , int ) ;
extern void DC_color_squeeze( MCW_DC * , int ) ;

extern void DC_palette_setgray( MCW_DC * ) ;
extern void DC_palette_setcolor( MCW_DC * ) ;

extern Boolean MCW_check_iconsize( int,int,MCW_DC * ) ;

extern XColor * DCpix_to_XColor( MCW_DC * , Pixel , int ) ;

extern void DC_fg_color( MCW_DC * , int ) ;
extern void DC_bg_color( MCW_DC * , int ) ;
extern void DC_fg_colortext( MCW_DC * , char * ) ;
extern void DC_linewidth( MCW_DC * , int ) ;
extern void DC_fg_colorpix( MCW_DC * , Pixel ) ;

extern void DC_linestyle( MCW_DC * , int ) ;
#define DC_solid_line(ddcc)  DC_linestyle((ddcc),LineSolid)
#define DC_dashed_line(ddcc) DC_linestyle((ddcc),LineOnOffDash)

extern void OVC_mostest( MCW_DCOV * ) ;

extern void DC_set_image_colors( MCW_DC * ) ;  /* 22 Aug 1998 */

extern void reload_DC_colordef( MCW_DC * ) ;   /* 11 Feb 1999 */
extern Pixel DC_rgb_to_pixel( MCW_DC *, byte,byte,byte ) ;
extern void DC_pixel_to_rgb( MCW_DC *, Pixel, byte *,byte *,byte * ) ;

extern Pixel DC_rgb_to_ovpix( MCW_DC *, byte,byte,byte ) ; /* 20 Dec 1999 */
extern void DC_rgb_to_ovrgb( MCW_DC *, int,int *,int,byte *, byte *, byte *) ;

#endif /* _MCW_DISPLAY_HEADER_ */
