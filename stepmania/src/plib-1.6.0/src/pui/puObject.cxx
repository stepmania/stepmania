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


#include "puLocal.h"

UL_RTTI_DEF1(puObject,puValue)


inline float clamp01 ( float x )
{
  return (x >= 1.0f) ? 1.0f : x ;
}

static void load_colour_scheme ( float col[][4], float r, float g,
                                                 float b, float a )
{
  puSetColour ( col [ PUCOL_FOREGROUND ], r, g, b, a ) ;
  puSetColour ( col [ PUCOL_BACKGROUND ], r/2.0f, g/2.0f, b/2.0f, a ) ;
  puSetColour ( col [ PUCOL_HIGHLIGHT  ], clamp01(r*1.3f), clamp01(g*1.3f),
                                             clamp01(b*1.3f), a ) ;

  if ( 4.0f * g + 3.0f * r + b > 4.0f )
  {
    puSetColour ( col [ PUCOL_LEGEND ], 0.0f, 0.0f, 0.0f, a ) ;
    puSetColour ( col [ PUCOL_MISC   ], 0.0f, 0.0f, 0.0f, a ) ;
  }
  else
  {
    puSetColour ( col [ PUCOL_LEGEND ], 1.0f, 1.0f, 1.0f, a ) ;
    puSetColour ( col [ PUCOL_MISC   ], 1.0f, 1.0f, 1.0f, a ) ;
  }
}


static int    defaultStyle = PUSTYLE_DEFAULT ;
static int    defaultBorderThickness = 5 ;
static puFont defaultLegendFont ;
static puFont defaultLabelFont  ;
static float  defaultColourScheme [ 4 ] ;

void puSetDefaultStyle ( int style )
{
  defaultStyle = style ;

  switch ( abs(style) )
  {
    case PUSTYLE_SPECIAL_UNDERLINED :
      defaultBorderThickness = 1 ;
      break ;

    case PUSTYLE_SMALL_BEVELLED :
    case PUSTYLE_SMALL_SHADED :
    case PUSTYLE_BOXED :
      defaultBorderThickness = 2 ;
      break ;

    case PUSTYLE_BEVELLED :
    case PUSTYLE_SHADED :
    case PUSTYLE_DROPSHADOW :
      defaultBorderThickness = 5 ;
      break ;
  }
}
int  puGetDefaultStyle ( void ) { return defaultStyle ; }

void puSetDefaultBorderThickness ( int t ) { defaultBorderThickness = t ;    }
int  puGetDefaultBorderThickness ( void )  { return defaultBorderThickness ; }

void puSetDefaultFonts ( puFont legendFont, puFont labelFont )
{
  defaultLegendFont = legendFont ;
  defaultLabelFont  = labelFont  ;
}

puFont puGetDefaultLabelFont  ( void ) { return defaultLabelFont  ; }
puFont puGetDefaultLegendFont ( void ) { return defaultLegendFont ; }

void puGetDefaultFonts ( puFont *legendFont, puFont *labelFont )
{
  if ( legendFont ) *legendFont = defaultLegendFont ;
  if ( labelFont  ) *labelFont  = defaultLabelFont  ;
}

void puSetDefaultColourScheme ( float r, float g, float b, float a )
{
  defaultColourScheme[0] = r ;
  defaultColourScheme[1] = g ;
  defaultColourScheme[2] = b ;
  defaultColourScheme[3] = a ;
  load_colour_scheme ( _puDefaultColourTable, r, g, b, a ) ;
}

void puGetDefaultColourScheme ( float *r, float *g, float *b, float *a )
{
  if ( r ) *r = defaultColourScheme[0] ;
  if ( g ) *g = defaultColourScheme[1] ;
  if ( b ) *b = defaultColourScheme[2] ;
  if ( a ) *a = defaultColourScheme[3] ;
}



void puObject::setColourScheme ( float r, float g, float b, float a )
{
  load_colour_scheme ( colour, r, g, b, a ) ;
  puPostRefresh () ;
}

puObject::puObject ( int minx, int miny, int maxx, int maxy ) : puValue ()
{
  type |= PUCLASS_OBJECT ;

  bbox.min[0] = abox.min[0] = minx ;
  bbox.min[1] = abox.min[1] = miny ;
  bbox.max[0] = abox.max[0] = maxx ;
  bbox.max[1] = abox.max[1] = maxy ;

  active_mouse_edge = PU_UP ;
  style      = defaultStyle ;
  visible = active  = TRUE  ;
  highlighted       = FALSE ;
  am_default        = FALSE ;
  window            = puGetWindow () ;
  v_status          = 0 ;
  isSubWidget       = FALSE ;

  cb          = NULL ;
  active_cb   = NULL ;
  down_cb     = NULL ;
  r_cb        = NULL ;
  border_thickness = defaultBorderThickness ;
  render_data = NULL ;
  user_data   = NULL ;
  next = prev = NULL ;
  label       = NULL ;
  labelPlace  = PUPLACE_LABEL_DEFAULT ;
  labelFont   = defaultLabelFont  ;
  legend      = NULL ;
  legendFont  = defaultLegendFont ;
  legendPlace = PUPLACE_LEGEND_DEFAULT ;

  for ( int i = 0 ; i < PUCOL_MAX ; i++ )
    puSetColour ( colour[i], _puDefaultColourTable[i] ) ;

  parent = NULL ;

  if ( ! puNoGroup() )
    puGetCurrGroup() -> add ( this ) ;

  puPostRefresh () ;
}


puObject::~puObject ()
{
  if ( parent != this && parent != NULL )
    parent -> remove ( this ) ;

  if ( this == puActiveWidget () )
    puDeactivateWidget () ;

  puPostRefresh () ;
}

void puObject::recalc_bbox ( void )
{
  bbox = abox ;

  if ( label != NULL )
  {
    switch ( labelPlace )  // Extend the bounding box left and right
    {
    case PUPLACE_ABOVE_LEFT    :
    case PUPLACE_UPPER_LEFT    :
    case PUPLACE_CENTERED_LEFT :
    case PUPLACE_LOWER_LEFT    :
    case PUPLACE_BELOW_LEFT    :
      bbox.min[0] -= labelFont.getStringWidth ( label ) + PUSTR_LGAP ;
      break ;

    case PUPLACE_ABOVE_RIGHT    :
    case PUPLACE_UPPER_RIGHT    :
    case PUPLACE_CENTERED_RIGHT :
    case PUPLACE_LOWER_RIGHT    :
    case PUPLACE_BELOW_RIGHT    :
      bbox.max[0] += labelFont.getStringWidth ( label ) + PUSTR_RGAP ;
      break ;
    }

    switch ( labelPlace )  // Extend the bounding box up and down
    {
    case PUPLACE_ABOVE_LEFT   :
    case PUPLACE_TOP_LEFT     :
    case PUPLACE_TOP_CENTERED :
    case PUPLACE_TOP_RIGHT    :
    case PUPLACE_ABOVE_RIGHT  :
      bbox.max[1] += labelFont.getStringHeight ( label ) + labelFont.getStringDescender () +
                     PUSTR_TGAP ;
      break ;

    case PUPLACE_BELOW_LEFT      :
    case PUPLACE_BOTTOM_LEFT     :
    case PUPLACE_BOTTOM_CENTERED :
    case PUPLACE_BOTTOM_RIGHT    :
    case PUPLACE_BELOW_RIGHT     :
      bbox.min[1] -= labelFont.getStringHeight ( label ) + labelFont.getStringDescender () +
                     PUSTR_BGAP ;
      break ;
    }
  }

  if ( parent != NULL )
    parent -> recalc_bbox () ;
}

void puObject::getAbsolutePosition ( int *x, int *y ) const
{
  puGroup *par ;
  *x = abox.min[0] ;
  *y = abox.min[1] ;

  for ( par = getParent () ; par != NULL ; par = par -> getParent () )
  {
    int x_offset, y_offset ;
    par -> getPosition ( &x_offset, &y_offset ) ;

    *x += x_offset ;
    *y += y_offset ;
  }
}

void puObject::draw_legend ( int dx, int dy )
{
  if ( legend == NULL )
    return ;

  int xx, yy ;

  int lgap = PUSTR_LGAP ;
  int rgap = PUSTR_RGAP ;
  int tgap = PUSTR_TGAP ;
  int bgap = PUSTR_BGAP ;

  if ( ( abs(style) != PUSTYLE_NONE ) &&
       ( abs(style) != PUSTYLE_PLAIN ) &&
       ( abs(style) != PUSTYLE_DROPSHADOW ) )
  {
    if ( abs(style) != PUSTYLE_SPECIAL_UNDERLINED )
    {
      lgap += getBorderThickness () ;
      rgap += getBorderThickness () ;
      tgap += getBorderThickness () ;
    }
    bgap += getBorderThickness () ;
  }

  /* If greyed out then halve the opacity when drawing the legend */

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

  switch ( getLegendPlace() )
  {
  case PUPLACE_TOP_LEFT      :
  case PUPLACE_CENTERED_LEFT :
  case PUPLACE_BOTTOM_LEFT   :
  case PUPLACE_LOWER_LEFT    :
    xx = lgap ;
    break ;

  case PUPLACE_TOP_CENTERED      :
  case PUPLACE_CENTERED_CENTERED :
  case PUPLACE_BOTTOM_CENTERED   :
    xx = ( abox.max[0] - abox.min[0] - legendFont.getStringWidth (legend) ) / 2 ;
    break ;

  case PUPLACE_TOP_RIGHT      :
  case PUPLACE_CENTERED_RIGHT :
  case PUPLACE_BOTTOM_RIGHT   :
  case PUPLACE_LOWER_RIGHT    :
    xx = abox.max[0] - abox.min[0] - legendFont.getStringWidth ( legend ) - rgap ;
    break ;

  default :
     ulSetError ( UL_WARNING, "PUI: Unrecognised LEGEND place %d", getLegendPlace() ) ;
     return ;
  }

  switch ( getLegendPlace() )
  {
  case PUPLACE_TOP_LEFT     :
  case PUPLACE_TOP_CENTERED :
  case PUPLACE_TOP_RIGHT    :
    yy = abox.max[1] - abox.min[1] - legendFont.getStringHeight ( legend ) -
         legendFont.getStringDescender () - tgap ;
    break ;

  case PUPLACE_CENTERED_LEFT     :
  case PUPLACE_CENTERED_CENTERED :
  case PUPLACE_CENTERED_RIGHT    :
  case PUPLACE_LOWER_LEFT        : /* Backwards compatibility to PUPLACE_LEFT */
  case PUPLACE_LOWER_RIGHT       : /* Backwards compatibility to PUPLACE_RIGHT */
  default :
    yy = ( abox.max[1] - abox.min[1] +
      legendFont.getStringHeight ( legend ) ) / 2 -
      legendFont.getStringHeight () ;
    break ;

  case PUPLACE_BOTTOM_LEFT     :
  case PUPLACE_BOTTOM_CENTERED :
  case PUPLACE_BOTTOM_RIGHT    :
    yy = bgap + legendFont.getStringDescender () ;
    break ;
  }

  legendFont.drawString ( legend, dx + abox.min[0] + xx,
                                  dy + abox.min[1] + yy ) ;
}

void puObject::draw_label ( int dx, int dy )
{
  if ( label == NULL )
    return ;

  int xx, yy ;

  /* If greyed out then halve the opacity when drawing the label */

  if ( active )
    glColor4fv ( colour [ PUCOL_LABEL ] ) ;
  else
    glColor4f ( colour [ PUCOL_LABEL ][0],
                colour [ PUCOL_LABEL ][1],
                colour [ PUCOL_LABEL ][2],
                colour [ PUCOL_LABEL ][3] / 2.0f ) ; /* 50% more transparent */

  switch ( getLabelPlace() )
  {
  case PUPLACE_ABOVE_LEFT    :
  case PUPLACE_UPPER_LEFT    :
  case PUPLACE_CENTERED_LEFT :
  case PUPLACE_LOWER_LEFT    :
  case PUPLACE_BELOW_LEFT    :
    xx = 0 ;
    break ;

  case PUPLACE_TOP_LEFT    :
  case PUPLACE_BOTTOM_LEFT :
    xx = abox.min[0] - bbox.min[0] + PUSTR_LGAP ;
    break ;

  case PUPLACE_TOP_CENTERED    :
  case PUPLACE_BOTTOM_CENTERED :
    xx = ( bbox.max[0] - bbox.min[0] - labelFont.getStringWidth ( label ) ) / 2 ;
    break ;

  case PUPLACE_TOP_RIGHT    :
  case PUPLACE_BOTTOM_RIGHT :
    xx = abox.max[0] - bbox.min[0] - labelFont.getStringWidth ( label ) - PUSTR_RGAP ;
    break ;

  case PUPLACE_ABOVE_RIGHT    :
  case PUPLACE_UPPER_RIGHT    :
  case PUPLACE_CENTERED_RIGHT :
  case PUPLACE_LOWER_RIGHT    :
  case PUPLACE_BELOW_RIGHT    :
    xx = bbox.max[0] - bbox.min[0] - labelFont.getStringWidth ( label ) ;
    break ;

  default :
     ulSetError ( UL_WARNING, "PUI: Unrecognised LABEL place %d", getLegendPlace() ) ;
     return ;
  }

  switch ( getLabelPlace() )
  {
  case PUPLACE_ABOVE_LEFT   :
  case PUPLACE_TOP_LEFT     :
  case PUPLACE_TOP_CENTERED :
  case PUPLACE_TOP_RIGHT    :
  case PUPLACE_ABOVE_RIGHT  :
    yy = bbox.max[1] - bbox.min[1] - labelFont.getStringHeight () -
         labelFont.getStringDescender () ;
    break ;

  case PUPLACE_UPPER_LEFT  :
  case PUPLACE_UPPER_RIGHT :
    yy = abox.max[1] - bbox.min[1] - labelFont.getStringHeight ( label ) - PUSTR_TGAP ;
    break ;

  case PUPLACE_CENTERED_LEFT  :
  case PUPLACE_CENTERED_RIGHT :
  default :
    yy = ( bbox.max[1] - bbox.min[1] +
      labelFont.getStringHeight ( label ) ) / 2 -
      labelFont.getStringHeight () ;
    break ;

  case PUPLACE_LOWER_LEFT  :
  case PUPLACE_LOWER_RIGHT :
    yy = abox.min[1] - bbox.min[1] + labelFont.getStringDescender () + PUSTR_BGAP ;
    break ;

  case PUPLACE_BELOW_LEFT      :
  case PUPLACE_BOTTOM_LEFT     :
  case PUPLACE_BOTTOM_CENTERED :
  case PUPLACE_BOTTOM_RIGHT    :
  case PUPLACE_BELOW_RIGHT     :
    yy = labelFont.getStringDescender () ;
    break ;
  }

  labelFont.drawString ( label, dx + bbox.min[0] + xx,
                                dy + bbox.min[1] + yy ) ;
}


int puObject::checkKey ( int key, int updown )
{
  if ( updown == PU_UP )
    return FALSE ;

  if ( isReturnDefault() && ( key == '\r' || key == '\n' ) && ( window == puGetWindow () ) )
  {
    if ( puActiveWidget() && ( this != puActiveWidget() ) )
    {
      puActiveWidget() -> invokeDownCallback () ;
      puDeactivateWidget () ;
    }

    checkHit ( PU_LEFT_BUTTON, PU_DOWN, (abox.min[0]+abox.max[0])/2,
                                        (abox.min[1]+abox.max[1])/2 ) ;
    checkHit ( PU_LEFT_BUTTON, PU_UP  , (abox.min[0]+abox.max[0])/2,
                                        (abox.min[1]+abox.max[1])/2 ) ;
    return TRUE ;
  }

  return FALSE ;
}


void puObject::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}

int puObject::checkHit ( int button, int updown, int x, int y )
{
  if ( ( ( puGetPressedButton () || updown == PU_UP ) ) && isHit( x, y ) )
  {
    doHit ( button, updown, x, y ) ;
    return TRUE ;
  }

  return FALSE ;
}


