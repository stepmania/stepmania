/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/

#ifndef _PU_H_
#define _PU_H_ 1

#include "fnt.h"
#include "ulRTTI.h"

/*
  Configuration
*/

#ifndef PU_NOT_USING_GLUT
#define _PU_USE_GLUT   1
#define _PU_USE_GLUT_FONTS   1
#endif

#define PU_NOBUTTON             -1
#define PU_LEFT_BUTTON          0
#define PU_MIDDLE_BUTTON        1
#define PU_RIGHT_BUTTON         2
#define PU_DOWN                 0
#define PU_UP                   1

#ifdef _PU_USE_GLUT_FONTS
typedef void *GlutFont ;
#else
extern fntTexFont PUFONT_TXF_TYPEWRITER ;
extern fntTexFont PUFONT_TXF_TIMES ;
extern fntTexFont PUFONT_TXF_HELVETICA ;
#endif

class puFont 
{
protected:
#ifdef _PU_USE_GLUT_FONTS
  GlutFont     glut_font_handle ;
#endif
  fntTexFont * fnt_font_handle ; float pointsize ; float slant ;

public:

  puFont () ;

#ifdef _PU_USE_GLUT_FONTS
  puFont ( GlutFont gfh )
  {
    glut_font_handle = gfh  ;
    fnt_font_handle  = NULL ;
  }
#endif

  puFont ( fntTexFont *tfh, float ps, float sl = 0 )
  {
    initialize ( tfh, ps, sl ) ;
  }

  void initialize ( fntTexFont *tfh, float ps, float sl = 0 )
  {
#ifdef _PU_USE_GLUT_FONTS
    glut_font_handle = (GlutFont) 0 ;
#endif
    fnt_font_handle  = tfh  ;
    pointsize = ps ;
    slant = sl ;
  }

  int getStringDescender ( void ) const ;
  int getStringHeight ( const char *str ) const ;
  int getStringHeight ( void ) const { return getStringHeight ( "K" ) ; }

  float getFloatStringWidth ( const char *str ) const ; 
  int   getStringWidth      ( const char *str ) const /* Deprecated ? */ 
  { 
    return (int) getFloatStringWidth ( str ) ; 
  } 

  void drawString ( const char *str, int x, int y ) ;
} ;


extern puFont PUFONT_8_BY_13        ;
extern puFont PUFONT_9_BY_15        ;
extern puFont PUFONT_TIMES_ROMAN_10 ;
extern puFont PUFONT_TIMES_ROMAN_24 ;
extern puFont PUFONT_HELVETICA_10   ;
extern puFont PUFONT_HELVETICA_12   ;
extern puFont PUFONT_HELVETICA_18   ;

#define PU_UP_AND_DOWN   254
#define PU_DRAG          255
#define PU_CONTINUAL     PU_DRAG

#define PU_KEY_GLUT_SPECIAL_OFFSET  256

#define PU_KEY_F1        (1             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F2        (2             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F3        (3             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F4        (4             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F5        (5             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F6        (6             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F7        (7             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F8        (8             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F9        (9             + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F10       (10            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F11       (11            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_F12       (12            + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_LEFT      (100           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_UP        (101           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_RIGHT     (102           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_DOWN      (103           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_UP   (104           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_PAGE_DOWN (105           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_HOME      (106           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_END       (107           + PU_KEY_GLUT_SPECIAL_OFFSET)
#define PU_KEY_INSERT    (108           + PU_KEY_GLUT_SPECIAL_OFFSET)

#define PUARROW_UP         0
#define PUARROW_DOWN       1
#define PUARROW_FASTUP     2
#define PUARROW_FASTDOWN   3
#define PUARROW_LEFT       4
#define PUARROW_RIGHT      5
#define PUARROW_FASTLEFT   6
#define PUARROW_FASTRIGHT  7

#define PUBUTTON_NORMAL 0
#define PUBUTTON_RADIO  1
#define PUBUTTON_CIRCLE 2
#define PUBUTTON_VCHECK 3 /* v-shaped checkmark */
#define PUBUTTON_XCHECK 4 /* X checkmark */

/* Rational Definitions of PUI Legend and Label Places */
#define PUPLACE_TOP_LEFT          0
#define PUPLACE_TOP_CENTERED      1
#define PUPLACE_TOP_RIGHT         2

#define PUPLACE_CENTERED_LEFT     3
#define PUPLACE_CENTERED_RIGHT    4

#define PUPLACE_BOTTOM_LEFT       5
#define PUPLACE_BOTTOM_CENTERED   6
#define PUPLACE_BOTTOM_RIGHT      7

/* Additional definitions for PUI Legend places */
#define PUPLACE_CENTERED_CENTERED 8

/* Additional definitions for PUI Label places */
#define PUPLACE_ABOVE_LEFT        9
#define PUPLACE_ABOVE_RIGHT      10

#define PUPLACE_BELOW_LEFT       11
#define PUPLACE_BELOW_RIGHT      12

#define PUPLACE_UPPER_LEFT       13
#define PUPLACE_UPPER_RIGHT      14

#define PUPLACE_LOWER_LEFT       15
#define PUPLACE_LOWER_RIGHT      16

/* Default places */
#define PUPLACE_LABEL_DEFAULT   PUPLACE_LOWER_RIGHT
#define PUPLACE_LEGEND_DEFAULT  PUPLACE_CENTERED_CENTERED

/* Keep these for backwards compatibility but deprecate them */
#define PUPLACE_ABOVE           PUPLACE_TOP_LEFT
#define PUPLACE_BELOW           PUPLACE_BOTTOM_LEFT
#define PUPLACE_LEFT            PUPLACE_LOWER_LEFT
#define PUPLACE_RIGHT           PUPLACE_LOWER_RIGHT
#define PUPLACE_CENTERED        PUPLACE_CENTERED_CENTERED
#define PUPLACE_TOP_CENTER      PUPLACE_TOP_CENTERED
#define PUPLACE_BOTTOM_CENTER   PUPLACE_BOTTOM_CENTERED
#define PUPLACE_LEFT_CENTER     PUPLACE_CENTERED_LEFT
#define PUPLACE_RIGHT_CENTER    PUPLACE_CENTERED_RIGHT

#define PUPLACE_DEFAULT         PUPLACE_LABEL_DEFAULT

#define PUCOL_FOREGROUND 0
#define PUCOL_BACKGROUND 1
#define PUCOL_HIGHLIGHT  2
#define PUCOL_LABEL      3
#define PUCOL_LEGEND     4
#define PUCOL_MISC       5
#define PUCOL_MAX        6

#define PUSLIDER_CLICK   0
#define PUSLIDER_ALWAYS  1
#define PUSLIDER_DELTA   2

/* These styles may be negated to get 'highlighted' graphics */

#define PUSTYLE_DEFAULT    PUSTYLE_SHADED
#define PUSTYLE_NONE       0
#define PUSTYLE_PLAIN      1
#define PUSTYLE_BEVELLED   2
#define PUSTYLE_BOXED      3
#define PUSTYLE_DROPSHADOW 4
#define PUSTYLE_SPECIAL_UNDERLINED 5
#define PUSTYLE_SMALL_BEVELLED     6
#define PUSTYLE_RADIO      7 /* deprecated ! */
#define PUSTYLE_SHADED     8
#define PUSTYLE_SMALL_SHADED   9
#define PUSTYLE_MAX        10

/* These are the gaps that we try to leave around text objects */

#define PUSTR_TGAP   5
#define PUSTR_BGAP   5
#define PUSTR_LGAP   5
#define PUSTR_RGAP   5

#define PU_RADIO_BUTTON_SIZE 16

extern int puRefresh ; /* Should not be used directly by applications any
                          longer. Instead, use puPostRefresh () and
                          puNeedRefresh (). */

#define PUCLASS_VALUE            0x00000001
#define PUCLASS_OBJECT           0x00000002
#define PUCLASS_GROUP            0x00000004
#define PUCLASS_INTERFACE        0x00000008
#define PUCLASS_FRAME            0x00000010
#define PUCLASS_TEXT             0x00000020
#define PUCLASS_BUTTON           0x00000040
#define PUCLASS_ONESHOT          0x00000080
#define PUCLASS_POPUP            0x00000100
#define PUCLASS_POPUPMENU        0x00000200
#define PUCLASS_MENUBAR          0x00000400
#define PUCLASS_INPUT            0x00000800
#define PUCLASS_BUTTONBOX        0x00001000
#define PUCLASS_SLIDER           0x00002000
#define PUCLASS_DIALOGBOX        0x00004000
#define PUCLASS_ARROW            0x00008000
#define PUCLASS_LISTBOX          0x00010000
#define PUCLASS_DIAL             0x00020000
#define PUCLASS_FILEPICKER       0x00040000
#define PUCLASS_FILESELECTOR     0x00040000 /* Because FilePicker is obsolete */
#define PUCLASS_BISLIDER         0x00080000
#define PUCLASS_TRISLIDER        0x00100000
#define PUCLASS_VERTMENU         0x00200000
#define PUCLASS_LARGEINPUT       0x00400000
#define PUCLASS_COMBOBOX         0x00800000
#define PUCLASS_SELECTBOX        0x01000000
#define PUCLASS_RANGE            0x02000000
#define PUCLASS_SPINBOX          0x04000000
#define PUCLASS_SCROLLBAR        0x08000000

/* This function is not required for GLUT programs */
void puSetWindowSize ( int width, int height ) ;
void puSetResizeMode ( int mode ) ;

int  puGetWindow       ( void ) ;
void puSetWindow       ( int w ) ;
int  puGetWindowHeight ( void ) ;
int  puGetWindowWidth  ( void ) ;

class puValue            ;
class puObject           ;
class puGroup            ;
class puInterface        ;
class puButtonBox        ;
class puFrame            ;
class puText             ;
class puButton           ;
class puOneShot          ;
class puPopup            ;
class puPopupMenu        ;
class puMenuBar          ;
class puInput            ;
class puRange            ;
class puSpinBox          ;
class puSlider           ;
class puListBox          ;
class puArrowButton      ;
class puDial             ;
class puFilePicker       ;
class puFileSelector     ;
class puBiSlider         ;
class puTriSlider        ;
class puVerticalMenu     ;
class puLargeInput       ;
class puComboBox         ;
class puSelectBox        ;
class puScrollBar        ;

// Global function to move active object to the end of the "dlist"
// so it is displayed in front of everything else

void puMoveToLast ( puObject *ob ) ;

typedef float puColour [ 4 ] ;  /* RGBA */
typedef puColour puColor ;

struct puBox
{
  int min [ 2 ] ;
  int max [ 2 ] ;

  void draw   ( int dx, int dy, int style, puColour colour[], int am_default, int border ) ;
  void extend ( puBox *bx ) ;

  void empty   ( void ) { min[0]=min[1]=1000000 ; max[0]=max[1]=-1000000 ; }
  int  isEmpty ( void ) const { return min[0]>max[0] || min[1]>max[1] ; }
} ;

#define PUSTRING_MAX 80

/* With many memory managers, allocating powers of two is more efficient */
// wk: This was 64. But in the viewer example, compiled as debug build with MSVC
// when you choose a file with a path/file name length more than 64, there will be
// a crash. Obviously, the right thing to do would be to find the real cause
// for this and fix it, but I could not find it and we need to release...
#define PUSTRING_INITIAL 128

extern puColour _puDefaultColourTable[] ;


inline void puSetColour ( puColour dst, puColour src )
{
  dst[0] = src[0] ; dst[1] = src[1] ; dst[2] = src[2] ; dst[3] = src[3] ;
}
inline void puSetColor ( puColour dst, puColour src )
{
  dst[0] = src[0] ; dst[1] = src[1] ; dst[2] = src[2] ; dst[3] = src[3] ;
}

inline void puSetColour ( puColour c, float r, float g, float b, float a = 1.0f )
{
  c [ 0 ] = r ; c [ 1 ] = g ; c [ 2 ] = b ; c [ 3 ] = a ;
}
inline void puSetColor ( puColour c, float r, float g, float b, float a = 1.0f )
{
  c [ 0 ] = r ; c [ 1 ] = g ; c [ 2 ] = b ; c [ 3 ] = a ;
}


void  puInit           ( void ) ;
void  puDisplay        ( void ) ;
void  puDisplay        ( int window_number ) ;
int   puMouse          ( int button, int updown, int x, int y ) ;
int   puMouse          ( int x, int y ) ;
int   puKeyboard       ( int key, int updown ) ;
void  puHideCursor     ( void ) ;
void  puShowCursor     ( void ) ;
int   puCursorIsHidden ( void ) ;
void  puDeleteObject   ( puObject *ob ) ;

int  puNeedRefresh     ( void ) ;
void puPostRefresh     ( void ) ;


// Active widget functions

void puDeactivateWidget ( void ) ;
void puSetActiveWidget ( puObject *w, int x, int y ) ;
puObject *puActiveWidget ( void ) ;

// Return the currently active mouse button
extern int puGetPressedButton () ;



class puValue
{
  UL_TYPE_DATA

protected:
  int   type    ;

  int   integer ;
  float floater ;
  char  *string ;

  int   *res_integer  ;
  float *res_floater  ;
  char  *res_string   ;

  int   string_size ;
  int   res_string_sz ;

  int   convert ;

  void re_eval    ( void ) ;
  void update_res ( void ) const { } /* Obsolete ! */ ;

  void copy_stringval ( const char *str ) ;

  int *   getIntegerp ( void ) { return res_integer != NULL ? res_integer : &integer ; }
  float * getFloaterp ( void ) { return res_floater != NULL ? res_floater : &floater ; }
  char *  getStringp  ( void ) { return res_string != NULL ? res_string : string ; }

  void enableConversion  ( void ) { convert = TRUE  ; }
  void disableConversion ( void ) { convert = FALSE ; }
  int  conversionEnabled ( void ) const { return convert ; }

public:
  puValue ()
  {
    convert = TRUE ;

    string_size = PUSTRING_INITIAL ;
    string = new char [ string_size ] ;

    type = PUCLASS_VALUE ;
    res_integer = NULL ;
    res_floater = NULL ;
    res_string  = NULL ;
    clrValue () ;
  }

  virtual ~puValue () { delete [] string ; }

  int  getType ( void ) const { return type ; }
  const char *getTypeString ( void ) const ;
  void clrValue ( void ) { setValue ( "" ) ; }

  virtual void setValue ( puValue *pv )
  {
    *getIntegerp () = pv -> getIntegerValue () ;
    *getFloaterp () = pv -> getFloatValue () ;
    copy_stringval ( pv -> getStringValue () ) ;
    puPostRefresh () ;
  }

  void setValuator ( int   *i )
  {
    res_integer = i ;

    if ( convert == TRUE )
    {
      res_floater = NULL ; res_string = NULL ;
      re_eval () ;
    }
  }

  void setValuator ( float *f )
  {
    res_floater = f ;

    if ( convert == TRUE )
    {
      res_integer = NULL ; res_string = NULL ;
      re_eval () ;
    }
  }

  void setValuator ( char *s, int size )
  {
    res_string = s ;
    res_string_sz = size ;

    if ( convert == TRUE )
    {
      res_integer = NULL ; res_floater = NULL ;
      re_eval () ;
    }
  }

  /* Obsolete ! */
  void setValuator ( char *s ) { setValuator ( s, PUSTRING_MAX ) ; }

  virtual void setValue ( int   i )
  {
    *getIntegerp () = i ;

    if ( convert == TRUE )
    {
      *getFloaterp () = (float) i ; sprintf ( getStringp (), "%d", i ) ;
    }

    puPostRefresh () ;
  }

  virtual void setValue ( float f )
  {
    *getFloaterp () = f ;

    if ( convert == TRUE )
    {
      *getIntegerp () = (int) f ; sprintf ( getStringp (), "%g", f ) ;
    }

    puPostRefresh () ;
  }

  virtual void setValue ( const char *s ) ;

  void getValue ( int   *i ) { re_eval () ; *i = *getIntegerp () ; }
  void getValue ( float *f ) { re_eval () ; *f = *getFloaterp () ; }
  void getValue ( char **s ) { re_eval () ; *s = getStringp ()   ; }
  void getValue ( char  *s, int size )
  {
    re_eval () ;

    /* Work around ANSI strncpy's null-fill behaviour */

    s[0] = '\0' ;
    strncat ( s, getStringp (), size-1 ) ;
  }

  void getValue ( char  *s ) { getValue ( s, PUSTRING_MAX ) ; } /* Obsolete ! */

  int  getValue ( void ) { return getIntegerValue () ; } /* Obsolete ! */

  int   getIntegerValue ( void ) { re_eval () ; return *getIntegerp () ; }
  float getFloatValue ( void )   { re_eval () ; return *getFloaterp () ; }
  char  getCharValue ( void )    { re_eval () ; return getStringp ()[0]; }
  char *getStringValue ( void )  { re_eval () ; return getStringp   () ; }

  /* RTTI */
  ulRTTItypeid getTypeInfo ( void ) const { return RTTI_vinfo () ; }
} ;

typedef void (*puCallback)(class puObject *) ;
typedef void (*puRenderCallback)(class puObject *, int dx, int dy, void *) ;

void puSetDefaultStyle ( int  style ) ;
int  puGetDefaultStyle ( void ) ;
void puSetDefaultBorderThickness ( int t ) ;
int  puGetDefaultBorderThickness ( void ) ;
void puSetDefaultFonts ( puFont  legendFont, puFont  labelFont ) ;
void puGetDefaultFonts ( puFont *legendFont, puFont *labelFont ) ;

puFont puGetDefaultLabelFont  ( void ) ;
puFont puGetDefaultLegendFont ( void ) ;

void puSetDefaultColourScheme ( float r, float g, float b, float a = 1.0f ) ;
inline void puSetDefaultColorScheme ( float r, float g, float b, float a = 1.0f )
{
  puSetDefaultColourScheme ( r, g, b, a ) ;
}

void puGetDefaultColourScheme ( float *r, float *g, float *b, float *a = NULL );
inline void puGetDefaultColorScheme ( float *r, float *g, float *b, float *a = NULL )
{
  puGetDefaultColourScheme ( r, g, b, a ) ;
}

class puObject : public puValue
{
  UL_TYPE_DATA

protected:
  puValue default_value ;

  puBox bbox ;   /* Bounding box of entire Object */
  puBox abox ;   /* Active (clickable) area */
  puColour colour [ PUCOL_MAX ] ;
  puGroup *parent ;

  int active_mouse_edge ; /* is it PU_UP or PU_DOWN (or both) that activates this? */
  int style       ;
  int visible     ;
  int active      ;
  int highlighted ;
  int am_default  ;
  int window ;        /* Which window does the object appear in? */
  int v_status ;      /* 1 if the Object should lock in the top left corner, 0 if not */
  bool isSubWidget;   /* TRUE if the Object is a child of another object but does not inherit from puGroup (SpinBoxes' puInput) */

  const char *label  ; puFont  labelFont ; int labelPlace ;
  const char *legend ; puFont legendFont ; int legendPlace ;

  void *user_data ;
  puCallback cb ;
  puCallback active_cb ;
  puCallback down_cb ;
  puRenderCallback r_cb ;
  void *render_data ;
  int border_thickness ;

  virtual void draw_legend ( int dx, int dy ) ;
  virtual void draw_label  ( int dx, int dy ) ;
  
public:
  virtual int  isHit ( int x, int y ) const { return isVisible() && isActive() &&
                                               x >= abox.min[0] &&
                                               x <= abox.max[0] &&
                                               y >= abox.min[1] &&
                                               y <= abox.max[1] &&
                                               window == puGetWindow () ; }

  virtual void doHit ( int button, int updown, int x, int y ) ;

  puObject ( int minx, int miny, int maxx, int maxy ) ;
  ~puObject () ;

  puObject *next ; /* Should not be used directly by applications any longer. */
  puObject *prev ; /* Instead, use the setNextObject and setPrevObject
                      methods. */
 
  puBox *getBBox ( void ) const { return (puBox *) &bbox ; }
  puBox *getABox ( void ) const { return (puBox *) &abox ; }

  void getAbsolutePosition ( int *x, int *y ) const ;

  virtual void setPosition ( int x, int y )
  {
    if ( abox.isEmpty() )
    {
      abox.max[0] = abox.min[0] = x ;
      abox.max[1] = abox.min[1] = y ;
    }
    else
    {
      abox.max[0] += x - abox.min[0] ;
      abox.max[1] += y - abox.min[1] ;
      abox.min[0]  = x ;
      abox.min[1]  = y ;
    }
    recalc_bbox() ; puPostRefresh () ;
  }

  virtual void setSize ( int w, int h )
  {
    abox.max[0] = abox.min[0] + w ;
    abox.max[1] = abox.min[1] + h ;
    recalc_bbox() ; puPostRefresh () ;
  }

  void getPosition ( int *x, int *y ) const
  {
    if ( abox.isEmpty () )
    {
      if ( x ) *x = 0 ;
      if ( y ) *y = 0 ;
    }
    else
    {
      if ( x ) *x = abox.min[0] ;
      if ( y ) *y = abox.min[1] ;
    }
  }

  void getSize ( int *w, int *h ) const
  {
    if ( abox.isEmpty () )
    {
      if ( w ) *w = 0 ;
      if ( h ) *h = 0 ;
    }
    else
    {
      if ( w ) *w = abox.max[0] - abox.min[0] ;
      if ( h ) *h = abox.max[1] - abox.min[1] ;
    }
  }

  virtual void recalc_bbox ( void ) ;
  virtual int  checkHit ( int button, int updown, int x, int y ) ;
  virtual int  checkKey ( int key   , int updown ) ;
  virtual void draw ( int dx, int dy ) = 0 ;

  puGroup     *getParent     ( void ) const { return parent ; }
  void        setParent      ( puGroup* p ) { parent = p ; }

  void        setNextObject  ( puObject *obj ) { next = obj  ; }
  puObject    *getNextObject ( void ) const    { return next ; }
  void        setPrevObject  ( puObject *obj ) { prev = obj  ; }
  puObject    *getPrevObject ( void ) const    { return prev ; }

  void       setCallback ( puCallback c ) { cb = c ;    }
  puCallback getCallback ( void ) const   { return cb ; }
  void       invokeCallback ( void ) { if ( cb ) (*cb)(this) ; }

  void       setActiveCallback ( puCallback c ) { active_cb = c ;    }
  puCallback getActiveCallback ( void ) const   { return active_cb ; }
  void       invokeActiveCallback ( void ) { if ( active_cb ) (*active_cb)(this) ; }

  void       setDownCallback ( puCallback c ) { down_cb = c ;    }
  puCallback getDownCallback ( void ) const   { return down_cb ; }
  virtual void invokeDownCallback ( void ) { if ( down_cb ) (*down_cb)(this) ; }

  void       setRenderCallback ( puRenderCallback c, void *d = NULL ) { r_cb = c ; render_data = d ; }
  puRenderCallback getRenderCallback ( void ) const { return r_cb ; }
  void      *getRenderCallbackData ( void ) const { return render_data ; }
  void       invokeRenderCallback ( int dx, int dy ) { if ( r_cb ) (*r_cb)(this, dx, dy, render_data) ; }

  void  setBorderThickness ( int t ) { border_thickness = t ; puPostRefresh () ; }
  int   getBorderThickness ( void ) const { return border_thickness ; }

  void  makeReturnDefault ( int def ) { am_default = def ; puPostRefresh () ; }
  int   isReturnDefault   ( void ) const { return am_default ; }

  int   getWindow ( void ) const { return window ; }
  void  setWindow ( int w ) { window = w ; puPostRefresh () ; }

  void  setActiveDirn ( int e ) { active_mouse_edge = e ; }
  int   getActiveDirn ( void ) const { return active_mouse_edge ; }

  void  setLegend ( const char *l ) { legend = l ; recalc_bbox() ; puPostRefresh () ; }
  const char *getLegend ( void ) const { return legend ; }

  void  setLegendFont ( puFont f ) { legendFont = f ; recalc_bbox() ; puPostRefresh () ; }
  puFont getLegendFont ( void ) const { return legendFont ; }

  void  setLegendPlace ( int lp ) { legendPlace = lp ; recalc_bbox() ; puPostRefresh () ; }
  int   getLegendPlace ( void ) const { return legendPlace ; }

  void  setLabel ( const char *l ) { label = l ; recalc_bbox() ; puPostRefresh () ; }
  const char *getLabel ( void ) const { return label ; }

  void  setLabelFont ( puFont f ) { labelFont = f ; recalc_bbox() ; puPostRefresh () ; }
  puFont getLabelFont ( void ) const { return labelFont ; }

  void  setLabelPlace ( int lp ) { labelPlace = lp ; recalc_bbox() ; puPostRefresh () ; }
  int   getLabelPlace ( void ) const { return labelPlace ; }

  void activate   ( void ) { if ( ! active  ) { active  = TRUE  ; puPostRefresh () ; } }
  void greyOut    ( void ) { if (   active  ) { active  = FALSE ; puPostRefresh () ; } }
  int  isActive   ( void ) const { return active ; }

  void highlight  ( void ) { if ( ! highlighted ) { highlighted = TRUE  ; puPostRefresh () ; } }
  void lowlight   ( void ) { if (   highlighted ) { highlighted = FALSE ; puPostRefresh () ; } }
  int isHighlighted( void ) const { return highlighted ; }

  void reveal     ( void ) { if ( ! visible ) { visible = TRUE  ; puPostRefresh () ; } }
  void hide       ( void ) { if (   visible ) { visible = FALSE ; puPostRefresh () ; } }
  int  isVisible  ( void ) const { return visible ; }

  bool IsItSubWidget( void ) { return isSubWidget ; }
  void ItIsSubWidget( void ) { isSubWidget = TRUE ; }

  void setStyle ( int which )
  {
    style = which ;

    switch ( abs(style) )
    {
      case PUSTYLE_SPECIAL_UNDERLINED :
        border_thickness = 1 ;
        break ;

      case PUSTYLE_SMALL_BEVELLED :
      case PUSTYLE_SMALL_SHADED :
      case PUSTYLE_BOXED :
        border_thickness = 2 ;
        break ;

      case PUSTYLE_BEVELLED :
      case PUSTYLE_SHADED :
      case PUSTYLE_DROPSHADOW :
        border_thickness = 5 ;
        break ;
    }

    recalc_bbox () ;
    puPostRefresh () ;
  }

  int  getStyle ( void ) const { return style ; }

  void setColourScheme ( float r, float g, float b, float a = 1.0f ) ;
  void setColorScheme ( float r, float g, float b, float a = 1.0f )
  {
    setColourScheme ( r, g, b, a ) ;
  }

  void setColour ( int which, float r, float g, float b, float a = 1.0f )
  {
    puSetColour ( colour [ which ], r, g, b, a ) ;
    puPostRefresh () ;
  }
  void setColor ( int which, float r, float g, float b, float a = 1.0f )
  {
    setColour ( which, r, g, b, a ) ;
  }

  void getColour ( int which, float *r, float *g, float *b, float *a = NULL ) const
  {
    if ( r ) *r = colour[which][0] ;
    if ( g ) *g = colour[which][1] ;
    if ( b ) *b = colour[which][2] ;
    if ( a ) *a = colour[which][3] ;
  }
  void getColor ( int which, float *r, float *g, float *b, float *a = NULL ) const
  {
    getColour ( which, r, g, b, a );
  }

  void  setUserData ( void *data ) { user_data = data ; }
  void *getUserData ( void ) const { return user_data ; }

  void defaultValue ( void ) { setValue ( & default_value ) ; }

  void setDefaultValue ( int    i ) { default_value.setValue ( i ) ; }
  void setDefaultValue ( float  f ) { default_value.setValue ( f ) ; }
  void setDefaultValue ( const char *s ) { default_value.setValue ( s ) ; }

  void getDefaultValue ( int   *i ) { default_value.getValue ( i ) ; }
  void getDefaultValue ( float *f ) { default_value.getValue ( f ) ; }
  void getDefaultValue ( char **s ) { default_value.getValue ( s ) ; }
  void getDefaultValue ( char  *s ) { default_value.getValue ( s ) ; }

  int  getDefaultValue ( void )     { return default_value.getValue () ; } /* Obsolete ! */

  int  getDefaultIntegerValue ( void ) { return default_value.getIntegerValue () ; }
  float getDefaultFloatValue  ( void ) { return default_value.getFloatValue   () ; }
  char *getDefaultStringValue ( void ) { return default_value.getStringValue  () ; }

  int getVStatus ( void ) const {  return v_status ;  } /* JCJ 6 Jun 2002 */
  void setVStatus ( int vstat ) {  v_status = vstat ;  }
} ;

/*
  The 'live' interface stack is used for clicking and rendering.
*/

void         puPushLiveInterface        ( puInterface *in ) ;
void         puPopLiveInterface         ( puInterface *in = 0 ) ;
int          puNoLiveInterface          ( void ) ;
puInterface *puGetBaseLiveInterface     ( void ) ;
puInterface *puGetUltimateLiveInterface ( void ) ;

/*
  The regular group stack is used for adding widgets
*/

void     puPushGroup    ( puGroup *in ) ;
void     puPopGroup     ( void ) ;
int      puNoGroup      ( void ) ;
puGroup *puGetCurrGroup ( void ) ;

class puGroup : public puObject
{
  UL_TYPE_DATA

protected:
  int num_children ;
  puObject *dlist ;

  int mouse_x ;    // Coordinates of mouse when right button pressed for
  int mouse_y ;    // drag and drop

  int mouse_active;  // Flag telling whether interface is presently being dragged

  void doHit       ( int button, int updown, int x, int y ) ;

  int floating;   // Flag telling whether the interface floats in the window or stays put

public:

  puGroup ( int x, int y ) : puObject ( x, y, x, y )
  {
    type |= PUCLASS_GROUP ;
    dlist = NULL ;
    num_children = 0 ;
    mouse_x = 0 ;
    mouse_y = 0 ;
    mouse_active = FALSE ;
    floating = FALSE ;
    puPushGroup ( this ) ;
  }

  ~puGroup () ;

  void recalc_bbox ( void ) ;
  virtual void add    ( puObject *new_object ) ;
  virtual void remove ( puObject *old_object ) ;
  virtual void empty ( void ) ;

  void draw        ( int dx, int dy ) ;
  int  checkHit    ( int button, int updown, int x, int y ) ;
  int  checkKey    ( int key   , int updown ) ;

  puObject *getFirstChild ( void ) const { return dlist ; }
  puObject *getLastChild  ( void ) const
  {
    puObject *bo = dlist ;

    if ( bo != NULL )
    {
      while ( bo -> getNextObject() != NULL )
        bo = bo -> getNextObject() ;
    }

    return bo ;
  }
  int getNumChildren ( void ) const { return num_children ; }

  virtual void close ( void )
  {
    if ( puGetCurrGroup () != this )
      ulSetError ( UL_WARNING, "PUI: puGroup::close() is mismatched!" ) ;
    else
      puPopGroup () ;
  }

  void setFloating ( int value ) { floating = value ; }
  int getFloating ( void ) const { return floating ; }

  void setChildStyle ( int childs, int which, int recursive = FALSE ) ;
  void setChildBorderThickness ( int childs, int t, int recursive = FALSE ) ;

  void setChildColour ( int childs, int which,
                        float r, float g, float b, float a = 1.0f,
                        int recursive = FALSE ) ;
  void setChildColor ( int childs, int which,
                       float r, float g, float b, float a = 1.0f,
                       int recursive = FALSE )
  {
    setChildColour ( childs, which, r, g, b, a, recursive ) ;
  }

  void setChildColourScheme ( int childs,
                              float r, float g, float b, float a = 1.0f,
                              int recursive = FALSE ) ;
  void setChildColorScheme ( int childs,
                             float r, float g, float b, float a = 1.0f,
                             int recursive = FALSE )
  {
    setChildColourScheme ( childs, r, g, b, a, recursive ) ;
  }

  void setChildLegendFont ( int childs, puFont f, int recursive = FALSE ) ;
  void setChildLabelFont ( int childs, puFont f, int recursive = FALSE ) ;
} ;


class puInterface : public puGroup
{
  UL_TYPE_DATA

public:

  puInterface ( int x, int y ) : puGroup ( x, y )
  {
    type |= PUCLASS_INTERFACE ;
    puPushLiveInterface ( this ) ;
  }

  ~puInterface () ;
} ;


class puFrame : public puObject
{
  UL_TYPE_DATA

public:
  void draw ( int dx, int dy ) ;
  puFrame ( int minx, int miny, int maxx, int maxy ) :
             puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_FRAME ;
  }

  void doHit ( int button, int updown, int x, int y )
  {
    if ( puActiveWidget() && ( this != puActiveWidget() ) )
    {
      // Active widget exists and is not this one; call its down callback if it exists

      puActiveWidget() -> invokeDownCallback () ;
      puDeactivateWidget () ;
    }

    if ( isHit ( x, y ) && ( updown != PU_DRAG ) )
      puMoveToLast ( this -> parent );
  }
} ;



class puText : public puObject
{
  UL_TYPE_DATA

public:
  virtual int  isHit ( int /* x */, int /* y */ ) const { return FALSE ; }
  void draw ( int dx, int dy ) ;
  puText ( int x, int y ) : puObject ( x, y, x, y )
  {
    type |= PUCLASS_TEXT ;
  }
} ;


class puButton : public puObject
{
  UL_TYPE_DATA

protected:
  int button_type ;

public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;

  int  getButtonType ( void ) const { return button_type ; }
  void setButtonType ( int btype )  { button_type = btype ; puPostRefresh () ; }

  puButton   ( int minx, int miny, const char *l ) :
                 puObject ( minx, miny,
                            minx + puGetDefaultLegendFont().getStringWidth ( l ) + PUSTR_LGAP + PUSTR_RGAP,
                            miny + puGetDefaultLegendFont().getStringHeight ( l ) + puGetDefaultLegendFont().getStringDescender () + PUSTR_TGAP + PUSTR_BGAP )
  {
    type |= PUCLASS_BUTTON ;
    button_type = PUBUTTON_NORMAL ;
    setLegend ( l ) ;
  }

  puButton   ( int minx, int miny, int maxx, int maxy, int btype = PUBUTTON_NORMAL ) :
                 puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_BUTTON ;
    button_type = btype ;
  }
} ;


class puOneShot : public puButton
{
  UL_TYPE_DATA

protected:
public:
  void doHit ( int button, int updown, int x, int y ) ;

  puOneShot ( int minx, int miny, const char *l ) : puButton   ( minx, miny, l )
  {
    type |= PUCLASS_ONESHOT ;
  }

  puOneShot ( int minx, int miny, int maxx, int maxy ) :
                 puButton ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_ONESHOT ;
  }
} ;



class puArrowButton : public puOneShot
{
  UL_TYPE_DATA

protected:
  int arrow_type ;

public:
  void draw  ( int dx, int dy ) ;

  int  getArrowType ( void  ) const { return arrow_type ; }
  void setArrowType ( int i ) { arrow_type = i ; puPostRefresh () ; }

  puArrowButton ( int minx, int miny, int maxx, int maxy, int ptype ) :
                 puOneShot ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_ARROW ;
    arrow_type = ptype ;
  }
} ;

class puRange : public puObject
{
  UL_TYPE_DATA

protected:
  float minimum_value ;
  float maximum_value ;
  float step_size ;
  float last_cb_value ;
  float cb_delta ;
  int   cb_mode ;

  void puRange_init ( float minval, float maxval, float step )
  {
    if ( maxval == minval )
    {
      maxval = 1.0 ;
      minval = 0.0 ;
    }

    minimum_value = minval ;
    maximum_value = maxval ;
    step_size = step ;
    getValue ( & last_cb_value ) ;  // was last_cb_value = -1.0f ;
    cb_delta = 0.1f ;
    cb_mode = PUSLIDER_ALWAYS ;
  }

public:
    puRange ( int minx, int miny, int maxx, int maxy ) :
     puObject ( minx, miny, maxx, maxy )
               {
                type |= PUCLASS_RANGE ;

                puRange_init ( 0.0, 1.0, 0.0 ) ;
               }

    puRange ( int minx, int miny, int maxx, int maxy, float minval, float maxval, float step = 0.0 ) :
     puObject ( minx, miny, maxx, maxy )
               {
                type |= PUCLASS_RANGE ;

                puRange_init ( minval, maxval, step ) ;
               }

  float getMaxValue ( void ) const { return maximum_value ; }
  virtual void setMaxValue ( float f ) { maximum_value = f ; }

  float getMinValue ( void ) const { return minimum_value ; }
  virtual void setMinValue ( float f ) { minimum_value = f ; }

  float getStepSize ( void ) { return step_size ; }
  void setStepSize ( float f ) { step_size = f ; }

  float checkStep ( float val_to_check )
  {
    float step = val_to_check ;

    if ( getStepSize() > 0.0 )
    {
      step = val_to_check - (float) fmod ( val_to_check, getStepSize() ) ;
      if ( ( val_to_check - step ) > ( step + getStepSize() - val_to_check ) ) step += getStepSize() ;
    }
      
    return step ;
  }
  
  void setCBMode ( int m ) { cb_mode = m ; }
  int getCBMode ( void ) const { return cb_mode ; }

  void setDelta ( float f ) { cb_delta = (f<=0.0f) ? 0.1f : (f>=1.0f) ? 0.9f : f ; }
  float getDelta ( void ) const { return cb_delta ; }
} ;

class puSlider : public puRange
{
  UL_TYPE_DATA

protected:
  int vert ;
  float slider_fraction ;
  void draw_slider_box ( int dx, int dy, const puBox &box, float val, const char *box_label = NULL ) ;

  void puSlider_init ( int vertical )
  {
    type |= PUCLASS_SLIDER ;
    slider_fraction = 0.1f ;
    vert = vertical ;
  }

public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puRange  ( minx, miny, vertical ?
               ( minx + puGetDefaultLegendFont().getStringWidth ( "W" ) +
                        PUSTR_LGAP + PUSTR_RGAP ) :
               ( minx + sz ),
              vertical ?
               ( miny + sz ) :
               ( miny + puGetDefaultLegendFont().getStringHeight () +
                        puGetDefaultLegendFont().getStringDescender () +
                        PUSTR_TGAP + PUSTR_BGAP )
              )
  {
    puSlider_init ( vertical ) ;
  }

  /* Blake Friesen - alternate constructor which lets you explicitly set width */
  puSlider ( int minx, int miny, int sz, int vertical, int width ) :
     puRange  ( minx, miny, vertical ?
                             ( minx + width ) :
                             ( minx + sz ),
                            vertical ?
                             ( miny + sz ) :
                             ( miny + width ) 
                            )
     { 
    puSlider_init ( vertical ) ;
  }
  
  int  isVertical ( void ) const { return vert ; }

  void setSliderFraction ( float f ) { slider_fraction = (f<=0.0f) ? 0.1f : (f>=1.0f) ? 0.9f : f ; puPostRefresh () ; }
  float getSliderFraction ( void ) const { return slider_fraction ; }
} ;


class puBiSlider : public puSlider
{
  UL_TYPE_DATA

protected:
  float current_max ;
  float current_min ;

  int active_button ;  // Zero for none, one for min, two for max
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puBiSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puSlider ( minx, miny, sz, vertical )
  {
    type |= PUCLASS_BISLIDER ;
    setMaxValue ( 1.0f ) ;
    setMinValue ( 0.0f ) ;
    setStepSize ( 1.0f ) ;
    current_max = 1.0 ;
    current_min = 0.0 ;
    active_button = 0 ;
  }

  /* Alternate constructor which lets you explicitly set width */

  puBiSlider ( int minx, int miny, int sz, int vertical, int width ) :
     puSlider ( minx, miny, sz, vertical, width )
  {
    type |= PUCLASS_BISLIDER ;
    setMaxValue ( 1.0f ) ;
    setMinValue ( 0.0f ) ;
    setStepSize ( 1.0f ) ;
    current_max = 1.0 ;
    current_min = 0.0 ;
    active_button = 0 ;
  }

  void setMaxValue ( float i )
  {
    maximum_value = i ;
    slider_fraction = 1.0f / ( getMaxValue() - getMinValue() + 1.0f ) ;
    puPostRefresh () ;
  }


  void setMinValue ( float i )
  {
    minimum_value = i ;
    slider_fraction = 1.0f / ( getMaxValue() - getMinValue() + 1.0f ) ;
    puPostRefresh () ;
  }

  void setCurrentMax ( int i ) { current_max = (float) i ; puPostRefresh () ; } /* DEPRECATED */
  void setCurrentMax ( float f ) { current_max = f ; puPostRefresh () ; }
  float getCurrentMax ( void ) const { return current_max ; }

  void setCurrentMin ( int i ) { current_min = (float) i ; puPostRefresh () ; } /* DEPRECATED */
  void setCurrentMin ( float f ) { current_min = f ; puPostRefresh () ; }
  float getCurrentMin ( void ) const { return current_min ; }

  void setActiveButton ( int i ) { active_button = i ; }
  int getActiveButton ( void ) const { return active_button ; }
} ;



class puTriSlider : public puBiSlider
{
  UL_TYPE_DATA

protected:
  // "active_button" is now zero for none, one for min, two for middle, three for max
  int freeze_ends ;  // true to make end sliders unmovable
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puTriSlider ( int minx, int miny, int sz, int vertical = FALSE ) :
     puBiSlider ( minx, miny, sz, vertical )
  {
    type |= PUCLASS_TRISLIDER ;
    freeze_ends = TRUE ;
  }

  /* Alternate constructor which lets you explicitly set width */

  puTriSlider ( int minx, int miny, int sz, int vertical, int width ) :
     puBiSlider ( minx, miny, sz, vertical, width )
  {
    type |= PUCLASS_TRISLIDER ;
    freeze_ends = TRUE ;
  }

  int getFreezeEnds ( void ) const { return freeze_ends ; }
  void setFreezeEnds ( int val )   { freeze_ends = val ; puPostRefresh () ; }
} ;


class puScrollBar : public puSlider
{
  UL_TYPE_DATA

protected:
  int arrow_count ;
  int fast_up_arrow_active, fast_down_arrow_active ;
  int up_arrow_active, down_arrow_active ;

public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puScrollBar ( int minx, int miny, int sz, int arrows, int vertical = FALSE ) :
     puSlider ( minx, miny, sz, vertical )
  {
    type |= PUCLASS_SCROLLBAR ;
    arrow_count = arrows ;
    fast_up_arrow_active = fast_down_arrow_active = up_arrow_active = down_arrow_active = FALSE ;
  }

  /* Alternate constructor which lets you explicitly set width */

  puScrollBar ( int minx, int miny, int sz, int arrows, int vertical, int width ) :
     puSlider ( minx, miny, sz, vertical, width )
  {
    type |= PUCLASS_SCROLLBAR ;
    arrow_count = arrows ;
    fast_up_arrow_active = fast_down_arrow_active = up_arrow_active = down_arrow_active = FALSE ;
  }

  void setMaxValue ( float f )
  {
    maximum_value = f ;
    slider_fraction = 1.0f / ( getMaxValue() - getMinValue() + 1.0f ) ;
    puPostRefresh () ;
  }


  void setMinValue ( float i )
  {
    minimum_value = i ;
    slider_fraction = 1.0f / ( getMaxValue() - getMinValue() + 1.0f ) ;
    puPostRefresh () ;
  }
} ;




class puListBox : public puButton
{
  UL_TYPE_DATA

protected:
  char ** list ;
  int num ;
  int top ;

public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;
  puListBox  ( int minx, int miny, int maxx, int maxy, char** list = NULL ) ;

  void newList     ( char ** _list ) ;
  int  getNumItems ( void ) const { return num ; }
  int  getNumVisible ( void ) const
  {
    int ysize = abox.max[1] - abox.min[1] + 1 ;
    int yinc = legendFont.getStringHeight () + PUSTR_BGAP ;
    return (ysize - PUSTR_BGAP) / yinc ;
  }

  int  getTopItem  ( void ) const { return top ; }
  void setTopItem  ( int item_index ) ;
} ;


class puDial : public puRange
{
  UL_TYPE_DATA

protected:
  int wrap ;  // Flag telling whether you can wrap around the bottom of the dial
public:
  void doHit ( int button, int updown, int x, int y ) ;
  void draw  ( int dx, int dy ) ;

  puDial ( int minx, int miny, int sz ) :
     puRange ( minx, miny, minx+sz, miny+sz )
  {
    type |= PUCLASS_DIAL ;
    setValue ( 0.0f ) ;
    wrap = TRUE ;
  }

  puDial ( int minx, int miny, int sz, float minval, float maxval, float step = 0.0 ) :
     puRange ( minx, miny, minx+sz, miny+sz, minval, maxval, step )
  {
    type |= PUCLASS_DIAL ;
    setValue ( 0.0f ) ;
    wrap = TRUE ;
  }

  void setWrap ( int in )    {  wrap = in ;  }
  int getWrap ( void ) const {  return wrap ;  }
} ;



class puPopup : public puInterface
{
  UL_TYPE_DATA

protected:
public:
  puPopup ( int x, int y ) : puInterface ( x, y )
  {
    type |= PUCLASS_POPUP ;
    hide () ;
  }
} ;


class puPopupMenu : public puPopup
{
  UL_TYPE_DATA

protected:
public:
  puPopupMenu ( int x, int y ) : puPopup ( x, y )
  {
    type |= PUCLASS_POPUPMENU ;
  }

  puObject *add_item ( const char *str, puCallback _cb,
                       void *_user_data = NULL ) ;
  int  checkHit ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key   , int updown ) ;
  void close ( void ) ;
} ;


class puMenuBar : public puInterface
{
  UL_TYPE_DATA

protected:
  int bar_height ;

public:
  puMenuBar ( int h = -1 ) : puInterface ( 0, 0 )
  {
    type |= PUCLASS_MENUBAR ;

    bar_height = h ;
  }

  void add_submenu ( const char *str, char *items[], puCallback _cb[],
                     void *_user_data[] = NULL ) ;
  void close ( void ) ;
} ;


class puVerticalMenu : public puGroup
{
  UL_TYPE_DATA

protected:
public:
  puVerticalMenu ( int x = -1, int y = -1 ) :

  puGroup ( x < 0 ? puGetWindowWidth() -
                     ( puGetDefaultLegendFont().getStringWidth ( " " )
                       + PUSTR_TGAP + PUSTR_BGAP ) : x,

          y < 0 ? puGetWindowHeight() -
                   ( puGetDefaultLegendFont().getStringHeight ()
                     + PUSTR_TGAP + PUSTR_BGAP ) : y)
  {
    type |= PUCLASS_VERTMENU ;
    floating = TRUE ;
    if ( y < 0 ) { setVStatus( TRUE ) ; } /* It is now supposed to stick to the top left - JCJ*/
  }

  void add_submenu ( const char *str, char *items[], puCallback _cb[],
                     void *_user_data[] = NULL ) ;
  void close ( void ) ;
} ;


class puInput : public puObject
{
  UL_TYPE_DATA

protected:
  int accepting ;
  int cursor_position ;
  int select_start_position ;
  int select_end_position ;
  char *valid_data ;

  virtual void normalize_cursors ( void ) ;
  virtual void removeSelectRegion ( void ) ;

  int input_disabled ;

public:
  void draw     ( int dx, int dy ) ;
  void doHit    ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key, int updown ) ;

  int  isAcceptingInput ( void ) const { return accepting ; }
  void rejectInput      ( void ) { accepting = FALSE ; puPostRefresh () ; }

  void acceptInput ( void )
  {
    accepting = TRUE ;
    cursor_position = strlen ( getStringValue() ) ;
    select_start_position = select_end_position = -1 ;
    puPostRefresh () ;
  }

  int  getCursor ( void ) const { return cursor_position ; }
  void setCursor ( int c ) { cursor_position = c ; }

  void setSelectRegion ( int s, int e )
  {
    select_start_position = s ;
    select_end_position   = e ;
    puPostRefresh () ;
  }

  void getSelectRegion ( int *s, int *e ) const
  {
    if ( s ) *s = select_start_position ;
    if ( e ) *e = select_end_position   ;
  }

  char *getValidData ( void ) const { return valid_data ; }
  void setValidData ( const char *data )
  {
    delete [] valid_data ;

    if ( data != NULL )
    {
      valid_data = new char [ strlen ( data ) + 1 ] ;
      strcpy ( valid_data, data ) ;
    }
    else
      valid_data = NULL ;
  }

  void addValidData ( const char *data )
  {
    int new_data_len = 1 ;
    if ( valid_data ) new_data_len += strlen ( valid_data ) ;
    if ( data )       new_data_len += strlen ( data ) ;
    char *new_data = new char [ new_data_len ] ;
    strcpy ( new_data, "\0" ) ;
    if ( valid_data ) strcat ( new_data, valid_data ) ;
    if ( data )       strcat ( new_data, data ) ;
    delete [] valid_data ;
    valid_data = new_data ;
  }

  int isValidCharacter ( char c ) const
  {
    if ( valid_data != NULL )
      return ( strchr ( valid_data, c ) != NULL ) ? 1 : 0 ;
    else
      return 1 ;
  }

  puInput ( int minx, int miny, int maxx, int maxy ) :
             puObject ( minx, miny, maxx, maxy )
  {
    type |= PUCLASS_INPUT ;

    accepting = FALSE ;

    cursor_position       =  0 ;
    select_start_position = -1 ;
    select_end_position   = -1 ;

    valid_data = NULL ;
    input_disabled = FALSE ;

    setColourScheme ( 0.8f, 0.7f, 0.7f ) ; /* Yeukky Pink */
    setColour ( PUCOL_MISC, 0.1f, 0.1f, 1.0f ) ; /* Colour of 'I' bar cursor */
  }

  ~puInput ()
  {
    delete [] valid_data ;
  }

  void invokeDownCallback ( void )
  {
    rejectInput () ;
    normalize_cursors () ;
    if ( down_cb ) (*down_cb)(this) ;
  }

  void enableInput ( void )  { input_disabled = FALSE ; }
  void disableInput ( void ) { input_disabled = TRUE  ; }
  int  inputDisabled ( void ) const { return input_disabled ; }
} ;

class puSpinBox : public puRange
{
  UL_TYPE_DATA

protected :
  puInput *input_box ;

  int up_arrow_active, down_arrow_active, arrow_position, inbox_height ;
  float arrow_height ;

public :
  int checkKey ( int key, int updown ) ;
  void doHit ( int button, int updown, int x, int y ) ;
  void draw ( int dx, int dy ) ;
  /* Whether the arrows are on the LEFT of the input box (0) or RIGHT (1 DEFAULT) */
  int getArrowPosition ( void ) { return arrow_position ; }
  /* Offered as a proportion of the input box height. Default = 0.5 */
  void setArrowHeight (float height ) { 
      puBox *ibox = input_box->getABox() ;
      abox.min[1] = ibox->min[1] ;
      abox.max[1] = ibox->max[1] ;
      abox.max[0] = ibox->max[0] ;
      arrow_height = height ; 
      abox.min[1] -= int(inbox_height * (getArrowHeight() - 0.5)) ;
      abox.max[1] += int(inbox_height * (getArrowHeight() - 0.5)) ;
      /* Push the right side out by scalar to ensure all the arrow area can be clicked */
      if (arrow_position == 0)
      {
        int size = int(inbox_height * arrow_height) ;
        int ih = ibox->max[1]-ibox->min[1] , ix = 0, iy = 0 ;
        input_box->getPosition(&ix, &iy);
        input_box->setPosition(abox.min[0] + size, iy) ;
        input_box->setSize(abox.max[0]-abox.min[0] - size, ih) ;
      } else {
        abox.max[0] += int(inbox_height * (getArrowHeight() - 0.5) + inbox_height/2) ;
      }
      recalc_bbox() ;
  }

  float getArrowHeight ( void ) { return arrow_height ; }

  puSpinBox ( int minx, int miny, int maxx, int maxy, int arrow_pos = 1 ) :
       puRange ( minx, miny, maxx, maxy, 1.0f, 10.0f, 1.0f )
  {
    extern void puSpinBox_handle_input(puObject* ob);
    type |= PUCLASS_SPINBOX ;
    if (arrow_pos==1)
        input_box = new puInput ( minx, miny, maxx - (maxy-miny)/2, maxy ) ;
    else
        input_box = new puInput ( minx + (maxy-miny)/2, miny, maxx, maxy ) ;
    up_arrow_active = down_arrow_active = 0 ;
    arrow_position = arrow_pos ;
    arrow_height = 0.5f ;
    inbox_height = maxy - miny ;
    input_box->setCallback(puSpinBox_handle_input) ;
    input_box->setDownCallback(puSpinBox_handle_input) ;
    input_box->setUserData(this) ;
    input_box->ItIsSubWidget() ;
  }

  ~puSpinBox ()
  {
    puDeleteObject ( (puObject *)input_box ) ;  // THIS MAY NEED TO CHANGE !!!
  }

  void setValue ( float f ) { puValue::setValue ( f ) ;  input_box->setValue ( f ) ; }
  void setValue ( int i ) { puValue::setValue ( i ) ;  input_box->setValue ( i ) ; }
  void setValue ( const char *s ) { puValue::setValue ( s ) ;  input_box->setValue ( s ) ; }
  void setValue ( puValue *pv ) { puValue::setValue ( pv ) ;  input_box->setValue ( pv ) ; }
} ;

class puButtonBox : public puObject
{
  UL_TYPE_DATA

protected:
  int one_only ;
  int num_kids ;
  char **button_labels ;

public:

  puButtonBox ( int minx, int miny, int maxx, int maxy, 
                char **labels, int one_button ) ;

  int isOneButton ( void ) const { return one_only ; }

  void newList     ( char ** _list ) ;
  int  getNumItems ( void ) const { return num_kids ; }

  int checkKey ( int key   , int updown ) ;
  int checkHit ( int button, int updown, int x, int y ) ;
  void draw    ( int dx, int dy ) ;
} ;


class puDialogBox : public puPopup
{
  UL_TYPE_DATA

protected:
public:

  puDialogBox ( int x, int y ) : puPopup ( x, y )
  {
    type |= PUCLASS_DIALOGBOX ;
  }
} ;



class puFilePicker : public puDialogBox
{
  UL_TYPE_DATA

protected:
  char** files ;
  char*  dflag ;
  int num_files   ;
  int arrow_count ;

  char startDir [ PUSTRING_MAX ] ;

  void find_files () ;
  static void handle_select ( puObject* ) ;
  static void input_entered ( puObject* ) ;

  puFrame   *frame         ;
  puListBox *list_box      ;
  puSlider  *slider        ;
  puOneShot *cancel_button ;
  puOneShot *ok_button     ;
  puArrowButton *up_arrow       ;
  puArrowButton *down_arrow     ;
  puArrowButton *fastup_arrow   ;
  puArrowButton *fastdown_arrow ;

  void puFilePickerInit ( int x, int y, int w, int h,
                          int arrows, const char *dir, const char *title ) ;

public:

/*******THIS CLASS IS OBSOLETE ********
  Please use puFileSelector - puFilePicker
  is obsolete and will be removed in a 
  future release.
 *************************************/

  puFilePicker ( int x, int y, int w, int h, int arrows,
                 const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFilePickerInit ( x, y, w, h, arrows, dir, title ) ;
  }

  puFilePicker ( int x, int y, int w, int h,
                 const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFilePickerInit ( x, y, w, h, 1, dir, title ) ;
  }

  puFilePicker ( int x, int y, int arrows,
                 const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFilePickerInit ( x, y, 220, 170, arrows, dir, title ) ;
  }

  puFilePicker ( int x, int y,
                 const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFilePickerInit ( x, y, 220, 170, 1, dir, title ) ;
  }

  ~puFilePicker () ;

  void setSize ( int w, int h ) ;
} ;


class puFileSelector : public puDialogBox
{
  UL_TYPE_DATA

protected:
  char** files ;
  char*  dflag ;
  int num_files   ;
  int arrow_count ;

  char *startDir ;

  void find_files ( void ) ;
  static void handle_select ( puObject* ) ;
  static void input_entered ( puObject* ) ;

  puFrame   *frame         ;
  puListBox *list_box      ;
  puSlider  *slider        ;
  puOneShot *cancel_button ;
  puOneShot *ok_button     ;
  puInput   *input         ;
  puArrowButton *up_arrow       ;
  puArrowButton *down_arrow     ;
  puArrowButton *fastup_arrow   ;
  puArrowButton *fastdown_arrow ;

  void puFileSelectorInit ( int x, int y, int w, int h,
                          int arrows, const char *dir, const char *title ) ;

public:

  puFileSelector ( int x, int y, int w, int h, int arrows,
                   const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFileSelectorInit ( x, y, w, h, arrows, dir, title ) ;
  }

  puFileSelector ( int x, int y, int w, int h,
                   const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFileSelectorInit ( x, y, w, h, 1, dir, title ) ;
  }

  puFileSelector ( int x, int y, int arrows,
                   const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFileSelectorInit ( x, y, 220, 170, arrows, dir, title ) ;
  }

  puFileSelector ( int x, int y,
                   const char *dir, const char *title = "Pick a file" ) :
     puDialogBox ( x, y )
  {
    puFileSelectorInit ( x, y, 220, 170, 1, dir, title ) ;
  }

  ~puFileSelector () ;

  /* Not for application use!! */
  puInput *__getInput ( void ) const { return input ; }
  char *__getStartDir ( void ) const { return (char *) startDir ; }

  void setInitialValue ( const char *fname ) ;
  void setSize ( int w, int h ) ;
} ;



class puLargeInput : public puInput
{
  UL_TYPE_DATA

protected:
  int num_lines ;              // Number of lines of text in the box
  int lines_in_window ;        // Number of lines showing in the window
  int top_line_in_window ;     // Number of the first line in the window
  float max_width ;            // Width of longest line of text in box, in pixels
  int slider_width ;

  puSlider *bottom_slider ;    // Horizontal slider at bottom of window
  puScrollBar *right_slider ;     // Vertical slider at right of window

  char *wrapped_text ;         // Pointer to word-wrapped text in the box

  int arrow_count ;          // Number of up/down arrows above and below the right slider

  void normalize_cursors ( void ) ;
  void removeSelectRegion ( void ) ;

  void wrapText ( void ) ;

public:
  puLargeInput ( int x, int y, int w, int h, int arrows, int sl_width, int wrap_text = FALSE ) ;
  ~puLargeInput () ;

  void setSize ( int w, int h ) ;

  int getNumLines ( void ) const {  return num_lines ;  }
  int getLinesInWindow ( void ) const { return lines_in_window ; }
  void setTopLineInWindow ( int val ) {  top_line_in_window = val ;  }

  void draw     ( int dx, int dy ) ;
  int  checkHit ( int button, int updown, int x, int y ) ;
  void doHit    ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key, int updown ) ;

  void setSelectRegion ( int s, int e ) ;
  void selectEntireLine ( void ) ;

  int  isAcceptingInput ( void ) const { return accepting ; }
  void rejectInput      ( void ) { accepting = FALSE ; puPostRefresh () ; }

  void acceptInput ( void )
  {
    accepting = TRUE ;
    cursor_position = strlen ( getStringValue() ) ;
    select_start_position = select_end_position = -1 ;
    puPostRefresh () ;
  }

  int  getCursor ( void ) const { return cursor_position ; }
  void setCursor ( int c ) { cursor_position = c ; puPostRefresh () ; }

  void getSelectRegion ( int *s, int *e ) const
  {
    if ( s ) *s = select_start_position ;
    if ( e ) *e = select_end_position   ;
  }

  void invokeDownCallback ( void )
  {
    rejectInput () ;
    normalize_cursors () ;
    if ( down_cb ) (*down_cb)(this) ;
  }

  void enableInput ( void )  { input_disabled = FALSE ; }
  void disableInput ( void ) { input_disabled = TRUE  ; }
  int  inputDisabled ( void ) const { return input_disabled ; }

  void  setValue ( const char *s ) ;
  void  setText ( const char *l ) { setValue ( l ) ; }  /* DEPRECATED */
  char *getText ( void ) { return getStringValue () ; }  /* DEPRECATED */
  char *getWrappedText ( void )
  {
    return ( wrapped_text == NULL ? getStringValue () : wrapped_text ) ;
  }
  void  addNewLine ( const char *l ) ;
  void  addText ( const char *l ) ;
  void  appendText ( const char *l ) ;
  void  removeText ( int start, int end ) ;
} ;


class puComboBox : public puGroup
{
  UL_TYPE_DATA

protected:
  char ** list  ;
  int num_items ;

  int curr_item ;

  puInput *input ;
  puArrowButton *arrow_btn ;
  puPopupMenu *popup_menu  ;

  static void input_down_cb ( puObject *inp   ) ;
  static void handle_arrow ( puObject *arrow  ) ;
  static void handle_popup ( puObject *popupm ) ;

  void update_widgets ( void ) ;

public:
  /* Not for application use ! */
  puPopupMenu * __getPopupMenu ( void ) const { return popup_menu ; }

  void newList ( char ** _list ) ;
  int  getNumItems ( void ) const { return num_items ; }

  int  getCurrentItem ( void ) ;
  void setCurrentItem ( int item )
  {
    if ( ( item >= 0 ) && ( item < num_items ) )
    {
      curr_item = item ;
      update_widgets () ;

      invokeCallback () ;
    }
  }
  void setCurrentItem ( const char *item_ptr ) ;

  void setPosition ( int x, int y )
  {
    puGroup::setPosition ( x, y ) ;

    /* Ensure that popup menu will show up at the right place */
    newList ( list ) ;
  }

  void draw ( int dx, int dy ) ;
  int  checkHit ( int button, int updown, int x, int y ) ;
  int  checkKey ( int key, int updown ) ;

  puComboBox ( int minx, int miny, int maxx, int maxy,
               char **list, int editable = TRUE ) ;
} ;


class puSelectBox : public puGroup
{
  UL_TYPE_DATA

protected:
  char ** list  ;
  int num_items ;

  int curr_item ;

  puInput *input ;
  puArrowButton *down_arrow ;
  puArrowButton *up_arrow   ;

  static void handle_arrow ( puObject *arrow ) ;

  void update_widgets ( void ) ;

public:
  void newList ( char ** _list ) ;
  int  getNumItems ( void ) const { return num_items ; }

  int  getCurrentItem ( void ) const { return curr_item ; }
  void setCurrentItem ( int item )
  {
    if ( ( item >= 0 ) && ( item < num_items ) )
    {
      curr_item = item ;
      update_widgets () ;

      invokeCallback () ;
    }
  }

  void setSize ( int w, int h ) ;
  void draw ( int dx, int dy ) ;
  int  checkKey ( int key, int updown ) ;

  puSelectBox ( int minx, int miny, int maxx, int maxy,
                char **list ) ;
} ;

#endif

