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

UL_RTTI_DEF1(puLargeInput,puInput)


// Callbacks from the internal widgets

static void puLargeInputHandleRightSlider ( puObject * slider )
{
  float val = ((puRange *)slider)->getMaxValue () - slider->getFloatValue () ;

  puLargeInput* text = (puLargeInput*) slider->getUserData () ;
  int lines_in_window = text->getLinesInWindow () ; 
  int num_lines = text->getNumLines () ;

  if ( num_lines > 0 )
  {
    int idx = int ( val + 0.5 ) ;
    if ( idx > ( num_lines - lines_in_window + 2 ) ) idx = num_lines - lines_in_window + 2 ;
    if ( idx < 0 ) idx = 0 ;

    text->setTopLineInWindow ( idx ) ;
  }
}

// Private function from the widget itself

void puLargeInput::normalize_cursors ( void )
{
  puInput::normalize_cursors () ;

  // Set the top line in the window so that the last line is at the bottom of the window

  if ( top_line_in_window > num_lines - lines_in_window + 2 )
    top_line_in_window = num_lines - lines_in_window + 2 ;

  if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
}

void puLargeInput::removeSelectRegion ( void )
{
  puInput::removeSelectRegion () ;

  wrapText () ;
}


// Public functions from the widget itself

puLargeInput::puLargeInput ( int x, int y, int w, int h, int arrows, int sl_width, int wrap_text ) :
puInput( x, y + (wrap_text?0:sl_width), x + w - sl_width, y + h )
{
  setColour ( PUCOL_MISC, 0.1f, 0.1f, 1.0f ) ; // Colour of the 'I' bar cursor

  // Set the variables

  type |= PUCLASS_LARGEINPUT ;
  num_lines = 1 ;
  slider_width = sl_width ;
  lines_in_window = ( h - (bottom_slider?slider_width:0) ) /
                    ( getLegendFont().getStringHeight() + getLegendFont().getStringDescender() + 1 ) ;
  top_line_in_window = 0 ;
  max_width = 0 ;

  accepting = FALSE ;
  cursor_position = 0 ;
  select_start_position = 0 ;
  select_end_position = -1 ;
  valid_data = NULL;

  // Set up the widgets

  if ( wrap_text )
    bottom_slider = (puSlider *)NULL ;
  else
  {
    bottom_slider = new puSlider ( x, y, w - slider_width, FALSE, slider_width ) ,
    bottom_slider->setValue ( 0.0f ) ;   // All the way to the left
//    bottom_slider->setDelta(0.1f); // Commented out CBModes and Deltas for these sliders to increase response time and to ensure the sliders react properly even when first selected - JCJ 13 Jun 2002
    bottom_slider->setSliderFraction (1.0f) ;
//    bottom_slider->setCBMode( PUSLIDER_DELTA );
  }

  right_slider = new puScrollBar ( x + w - slider_width, y + (bottom_slider?slider_width:0),
                                   h - (bottom_slider?slider_width:0), arrows, TRUE, slider_width ) ,
  right_slider->setValue ( 1.0f ) ;    // All the way to the top
//  right_slider->setDelta(0.1f);
  right_slider->setSliderFraction (1.0f) ;
  right_slider->setStepSize ( 1.0f ) ;
//  right_slider->setCBMode( PUSLIDER_DELTA );
  right_slider->setUserData ( this ) ;
  right_slider->setCallback ( puLargeInputHandleRightSlider ) ;

  input_disabled = FALSE ;

  wrapped_text = NULL ;
  setValue ( "\n" ) ;

  reveal () ;
}

puLargeInput::~puLargeInput ()
{
  delete [] wrapped_text ;

  if ( puActiveWidget() == this )
    puDeactivateWidget () ;
}

void puLargeInput::setSize ( int w, int h )
{
  // Resize and reposition the sliders
  if ( bottom_slider )
    bottom_slider->setSize ( w - slider_width, slider_width ) ;
  else  // No bottom slider, rewrap the text
    wrapText () ;

  puInput::setSize ( w - slider_width, h - (bottom_slider?slider_width:0) ) ;
  right_slider->setPosition ( abox.min[0]+w-slider_width, abox.min[1] ) ;
  right_slider->setSize ( slider_width, h-(bottom_slider?slider_width:0) ) ;

  lines_in_window = ( h - (bottom_slider?slider_width:0) ) /
                    ( getLegendFont().getStringHeight() + getLegendFont().getStringDescender() + 1 ) ;

  int line_size = legendFont.getStringHeight () +     // Height of a line
                  legendFont.getStringDescender() ;  // of text, in pixels
  int box_height = ( abox.max[1] - abox.min[1] - slider_width ) / line_size ;
  int right_slider_max = num_lines - lines_in_window + 1 ;
  if ( right_slider_max < 1 ) right_slider_max = 1 ;

  right_slider->setSliderFraction ( float(box_height) / float(right_slider_max) ) ;
  right_slider->setMaxValue ( float(right_slider_max) ) ;
}

void puLargeInput::setSelectRegion ( int s, int e )
{
  puInput::setSelectRegion ( s, e ) ;
  char *lin_ptr = ( bottom_slider ? getText () : getWrappedText () ) ;
  char *text_start = lin_ptr ;

  if ( num_lines > lines_in_window )
  {
    int select_start_line = 0 ;
    while ( lin_ptr && ( lin_ptr <= text_start + select_start_position ) )  // Count the lines
    {
      select_start_line++ ;
      lin_ptr = strchr ( lin_ptr+1, '\n' ) ;
    }

    int select_end_line = select_start_line ;
    while ( lin_ptr && ( lin_ptr <= text_start + select_end_position ) )  // Count the lines
    {
      select_end_line++ ;
      lin_ptr = strchr ( lin_ptr+1, '\n' ) ;
    }

    if ( select_end_line > top_line_in_window + lines_in_window )
      top_line_in_window = select_end_line - lines_in_window - 1 ;

    if ( select_start_line < top_line_in_window )
      top_line_in_window = select_start_line - 1 ;

    if ( top_line_in_window < 0 ) top_line_in_window = 0 ;

    right_slider->setValue ( 1.0f - float(top_line_in_window) / float(num_lines - lines_in_window) ) ;
  }
}

void  puLargeInput::selectEntireLine ( void )
{
  char *temp_text = ( bottom_slider ? getText () : getWrappedText () ) ;
   
  if ( select_start_position < 0 )
      select_start_position = 0 ;

  while ( ( select_start_position > 0 ) && ( *(temp_text + select_start_position) != '\n' ) )
    select_start_position -- ;

  if ( select_start_position > 0 )
    select_start_position++ ;

  select_end_position = int ( strchr ( temp_text + select_end_position, '\n' ) + 1 - temp_text) ;
  if ( select_end_position == 1 ) select_end_position = strlen ( temp_text ) ;
  //else select_end_position = int ( select_end_position - temp_text ) ;/** Needs real fixing **/

  puPostRefresh () ;
}

void  puLargeInput::addNewLine ( const char *l )
{
  char *text = getStringValue () ;
  if ( cursor_position > 0 )  // If not at start of line, go to start of next line
    cursor_position = int (strchr ( text + cursor_position - 1, '\n' ) - text + 1) ;

  select_end_position = select_start_position = cursor_position ;
  addText ( l ) ;
}

void  puLargeInput::addText ( const char *l )
{
  char *text = getStringValue () ;

  if ( !l ) return ;

  int length = strlen ( l ) + strlen ( text )  /* Length of the final string */
               + select_start_position - select_end_position + 2 ;
  if ( *(l+strlen(l)-1) == '\n' ) length -- ;           // Decrement "length" for each final
  if ( text[select_end_position] == '\n' ) length -- ;  // carriage return already there

  char *temp_text = new char [ length ] ;

  strncpy ( temp_text, text, select_start_position ) ;
  *(temp_text+select_start_position) = '\0' ;

  strcat ( temp_text, l ) ;
  if ( ( *(l+strlen(l)-1) == '\n' ) && ( text[select_end_position] == '\n' ) )
    temp_text[strlen(temp_text)-1] = '\0' ;  /* Erase the duplicate carriage return */
  else if ( ( *(l+strlen(l)-1) != '\n' ) && ( text[select_end_position] != '\n' ) )
    strcat ( temp_text, "\n" ) ;  /* Add a carriage return */

  strcat ( temp_text, (text+select_end_position) ) ;
  int temp_select_start = select_start_position ;
  setText ( temp_text ) ;
  delete [] temp_text ;
  setSelectRegion ( temp_select_start,
                    temp_select_start + strlen(l) ) ;
  setCursor ( select_end_position ) ;
}

void  puLargeInput::appendText ( const char *l )
{
  if ( !l ) return ;

  int oldlen = strlen ( getStringValue () ) ;
  if ( oldlen == 1 ) oldlen = 0 ;  /* Don't want null line at the beginning */
  int length = oldlen + strlen ( l ) + 2 ;
  if ( *(l+strlen(l)-1) == '\n' ) length -- ;  /* Already have a trailing carriage return, decrement the length */

  char *temp_text = new char [ length ] ;

  if ( oldlen > 0 )  /* More than just the empty carriage return */
    strcpy ( temp_text, getStringValue () ) ;
  else
    temp_text[0] = '\0' ;

  strcat ( temp_text, l ) ;
  if ( *(l+strlen(l)-1) != '\n' )
    strcat ( temp_text, "\n" ) ;

  setValue ( temp_text ) ;
  setSelectRegion ( oldlen, strlen(temp_text) ) ;
  setCursor ( oldlen ) ;
  delete [] temp_text ;
}

void  puLargeInput::removeText ( int start, int end )
{
  char *temp_text = new char [ strlen(getStringValue ()) + start - end + 1 ] ;
  strncpy ( temp_text, getStringValue (), start ) ;
  temp_text[start] = '\0' ;
  strcat ( temp_text, getStringValue ()+end ) ;
  setValue ( temp_text ) ;
  setCursor ( start ) ;
  setSelectRegion ( start, start ) ;
  delete [] temp_text ;
}

void  puLargeInput::setValue ( const char *s )
{
  cursor_position = 0 ;
  select_start_position = select_end_position = 0 ;

  if ( bottom_slider ) bottom_slider->setSliderFraction ( 0.0 ) ;
  right_slider->setSliderFraction ( 0.0 ) ;

  puPostRefresh () ;

  if ( !s )
  {
    puValue::setValue ( "\n" ) ;
    num_lines = 0 ;
    return ;
  }

  int length = strlen ( s ) + 2 ;
  if ( ( strlen(s) > 0 ) && ( *(s+strlen(s)-1) == '\n' ) )
    length -- ;  /* Already have a trailing carriage return, don't need to add one */

  char *text = new char [ length ] ;
  strcpy ( text, s ) ;
  if ( ( s [ 0 ] == '\0' ) || ( *(s+strlen(s)-1) != '\n' ) )
    strcat ( text, "\n" ) ;

  puValue::setValue ( text ) ;
  delete [] text ;

  // Find the greatest width of a line
  max_width = 0 ;

  float line_width = 0.0 ;       // Width of current line
  if ( !bottom_slider ) wrapText () ;
  char *this_char = ( bottom_slider ? getStringValue () : getWrappedText () ) ;   // Pointer to character in text

  num_lines = 0 ;

  while ( *this_char != '\0' )
  {
    char *line_end = strchr ( this_char, '\n' ) ;
    if ( line_end )  // Found an end-of-line
    {
      *line_end = '\0' ;  // Temporary break in line
      line_width = legendFont.getFloatStringWidth ( this_char ) ;
      *line_end = '\n' ;  // Reset the carriage return

      if ( max_width < line_width )
        max_width = line_width ;

      num_lines++ ;                 // Increment line counter

      this_char = line_end + 1 ;
    }
    else  // No carriage return.  Since a carriage return should be the last character,
      this_char++ ;        // we should not get here.
  }

  if ( max_width < line_width )
    max_width = line_width ;

  // Set slider fractions

  int line_size = legendFont.getStringHeight () +     // Height of a line
                  legendFont.getStringDescender() ;  // of text, in pixels

  int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
  int box_height = ( abox.max[1] - abox.min[1] - slider_width ) / line_size ;
                                                // Input box height, in lines

  if ( bottom_slider )
    bottom_slider->setSliderFraction ( float(box_width) / float(max_width) ) ;

  int right_slider_max = num_lines - lines_in_window + 1 ;
  if ( right_slider_max < 1 ) right_slider_max = 1 ;

  right_slider->setSliderFraction ( float(box_height) / float(right_slider_max) ) ;
  right_slider->setMaxValue ( float(right_slider_max) ) ;
}


void puLargeInput::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;
  normalize_cursors () ;

  // 3D Input boxes look nicest if they are always in inverse style.

  abox.draw ( dx, dy, ( (style==PUSTYLE_SMALL_BEVELLED ||
                         style==PUSTYLE_SMALL_SHADED) ) ? -style :
                        (accepting ? -style : style ), colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    // Calculate window parameters:

    int line_size = legendFont.getStringHeight () +         // Height of a line
                    legendFont.getStringDescender() + 1 ;  // of text, in pixels

    int xx = int(legendFont.getFloatStringWidth ( " " )) ;
    int yy = int( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 ) ;

    int box_width = abox.max[0] - abox.min[0] - slider_width - xx - xx ;   // Input box width, in pixels
    int box_height = ( abox.max[1] - abox.min[1] - (bottom_slider?slider_width:0) ) / line_size ;
                                                  // Input box height, in lines

    float bottom_value = bottom_slider ? bottom_slider->getFloatValue () : 0.0 ;

    int beg_pos      // Position in window of start of line, in pixels
                = int(( box_width - max_width ) * bottom_value ) ;
////  int end_pos      // Position in window of end of line, in pixels
////              = beg_pos + max_width - 1 ;
    if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
    int end_lin      // Position on line count of bottom of window, in lines
                = top_line_in_window + box_height ;

   /* Removed IF statement to permit highlighting to remain even when widget not active - JCJ 13 Jun 2002 */
    
    char *val = bottom_slider ? getStringValue () : getWrappedText () ;

    // Highlight the select area

    if ( select_end_position > 0 &&
          select_end_position != select_start_position )    
    {
       // First:  find the positions on the window of the selection start and end

       char temp_char = val[ select_start_position ] ;
       val [ select_start_position ] = '\0' ;

       xx = dx + abox.min[0] + int(legendFont.getFloatStringWidth ( " " )) ;
       yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 
               + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

       char *end_of_line = strchr ( val, '\n' ) ;
       char *start_of_line = val;

       // Step down the lines until you reach the line with the selection start

       int select_start_line = 0 ;

       while ( end_of_line )
       {
         select_start_line++ ;
         start_of_line = end_of_line + 1 ;
         yy -= line_size ;
         end_of_line = strchr ( start_of_line, '\n' ) ;
       }

       int start_pos = int(legendFont.getFloatStringWidth ( start_of_line )) + xx +
                       beg_pos ;   // Start of selection

       val [ select_start_position ] = temp_char ;

       // Now repeat the process for the end of the selection.

       temp_char = val[ select_end_position ] ;
       val [ select_end_position ] = '\0' ;

       end_of_line = strchr ( start_of_line, '\n' ) ;

       // Step down the lines until you reach the line with the selection end

       int select_end_line = select_start_line ;

       while ( end_of_line )
       {
         select_end_line++ ;
         start_of_line = end_of_line + 1 ;
         end_of_line = strchr ( start_of_line, '\n' ) ;
       }

       int end_pos = int(legendFont.getFloatStringWidth ( start_of_line )) + xx +
                     beg_pos ;   // End of selection

       val [ select_end_position ] = temp_char ;

       // Now draw the selection area.

       for ( int line_count = select_start_line ; ( ( line_count <= select_end_line ) && ( line_count <= end_lin ) ) ;
                 line_count++ )
       {
         if ( line_count >= top_line_in_window )
         {
           int x_start, x_end ;

           if ( line_count == select_start_line )
             x_start = ( start_pos > xx ) ? start_pos : xx ;
           else
             x_start = xx ;

           x_start = ( x_start < abox.max[0] + dx ) ? x_start : abox.max[0] + dx ;

           if ( line_count == select_end_line )
             x_end = ( end_pos < abox.max[0] + dx ) ? end_pos : abox.max[0] + dx ;
           else
             x_end = abox.max[0] + dx ;

           x_end = ( x_end > xx ) ? x_end : xx ;

           int top = dy + abox.min[1] + yy + legendFont.getStringHeight () ;
           int bot = dy + abox.min[1] + yy - legendFont.getStringDescender() ;

           glColor3f ( 1.0f, 1.0f, 0.7f ) ;
           glRecti ( x_start, bot, x_end, top ) ;
         }
         yy -= line_size ;

         if ( line_count == end_lin ) break ;
       }
     }
    

    // Draw the text

    {
      // If greyed out then halve the opacity when drawing the text

      if ( active )
        glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
      else
        glColor4f ( colour [ PUCOL_LEGEND ][0],
                    colour [ PUCOL_LEGEND ][1],
                    colour [ PUCOL_LEGEND ][2],
                    colour [ PUCOL_LEGEND ][3] / 2.0f ) ; // 50% more transparent

      char *val ;                   // Pointer to the actual text in the box
      val = bottom_slider ? getStringValue () : getWrappedText () ;

      if ( val )
      {
        char *end_of_line = strchr (val, '\n') ;
        int line_count = 0;

        xx = int(legendFont.getFloatStringWidth ( " " )) ;
        yy = int( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 ) ;

        while ( end_of_line )  // While there is a carriage return in the string
        {
          if ( line_count < top_line_in_window )
          {                                        // Before the start of the window
            val = end_of_line + 1 ;
            end_of_line = strchr (val, '\n') ;     // Just go to the next line
          }
          else if ( line_count <= end_lin )        // Within the window, draw it
          {
            char temp_char = *end_of_line ;   // Temporary holder for last char on line

            *end_of_line = '\0' ;     // Make end-of-line be an end-of-string

            int start_pos = beg_pos ;
            int end_pos      // Position in window of end of line, in pixels
                    = start_pos + (int)legendFont.getFloatStringWidth ( val ) ;

            if ( end_pos > start_pos )  // If we actually have text in the line
            {
                char * lastonleft = val ;
                char * firstonright = end_of_line ;
                int leftpos = start_pos ;
                int rightpos = end_pos ;
                while ( lastonleft < firstonright - 1 ) 
                {
                    int chpos = -leftpos * ( firstonright - lastonleft ) / ( rightpos - leftpos ) + 1;
                    if ( chpos >= firstonright - lastonleft ) chpos = firstonright - lastonleft - 1 ;
                    char placeholder = *(lastonleft + chpos) ;
                    *(lastonleft + chpos) = '\0' ;
                    int strwidth = legendFont.getStringWidth ( val ) ;
                    *(lastonleft + chpos) = placeholder ;
                    if ( strwidth + start_pos < 0 ) /* Still to the left of the window */
                    {
                        lastonleft = lastonleft + chpos ;
                        placeholder = *lastonleft ;
                        *lastonleft = '\0' ;
                        leftpos = start_pos + legendFont.getStringWidth ( val ) ;
                        *lastonleft = placeholder ;
                    }
                    else
                    {
                        firstonright = lastonleft + chpos ;
                        placeholder = *firstonright ;
                        *firstonright = '\0' ;
                        rightpos = start_pos + legendFont.getStringWidth ( val ) ;
                        *firstonright = placeholder ;
                    }
                }
                if ( leftpos >= 0 )
                {
                  val = lastonleft ;
                  start_pos = leftpos ;
                }
                else
                {
                  val = firstonright ;
                  start_pos = rightpos ;
                }
            }

            while ( end_pos > box_width )  // Step up the line until it is in the window
            {
              *end_of_line = temp_char ;
              end_of_line-- ;
              temp_char = *end_of_line ;
              *end_of_line = '\0' ;
              end_pos = start_pos + legendFont.getStringWidth ( val ) ;
            }

            if ( val < end_of_line )                 // If any text shows in the window,
              legendFont.drawString ( val,           // draw it.
                                      dx + abox.min[0] + xx + start_pos,
                                      dy + abox.min[1] + yy ) ;

            *end_of_line = temp_char ;     // Restore the end-of-line character

            if ( temp_char != '\n' )               // If we had to step up from the end of
              end_of_line = strchr (val, '\n') ;   // the line, go back to the actual end

            yy -= line_size ;
            val = end_of_line + 1 ;
            end_of_line = strchr (val, '\n') ;     // On to the next line
          }
          else if ( line_count > end_lin )        // Have gone beyond window, end process
            end_of_line = NULL ;

          line_count++ ;

        }     // while ( end_of_line )
      }     // if ( val )
    }

    if ( accepting )
    { 
      char *val ;                   // Pointer to the actual text in the box
      val = bottom_slider ? getStringValue () : getWrappedText () ;

      // Draw the 'I' bar cursor.

      if ( val && ( cursor_position >= 0 ) )
      {
        char temp_char = val[ cursor_position ] ;
        val [ cursor_position ] = '\0' ;

        xx = (int)legendFont.getFloatStringWidth ( " " ) ;
        yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 
                + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

        char *end_of_line = strchr ( val, '\n' ) ;
        char *start_of_line = val;

        // Step down the lines until you reach the line with the cursor

        int line_count = 1 ;

        while ( end_of_line )
        {
          line_count++ ;
          start_of_line = end_of_line + 1 ;
          yy -= line_size ;
          end_of_line = strchr ( start_of_line, '\n' ) ;
        }

        if ( ( line_count > top_line_in_window ) && ( line_count <= end_lin ) )
        {
          int begpos      // Position in window of start of line, in pixels
                    = int( ( box_width - max_width ) * bottom_value ) ;
          int cpos = int( legendFont.getFloatStringWidth ( start_of_line ) + xx +
                     abox.min[0] + begpos ) ;
          int top = int( abox.min[1] + yy + legendFont.getStringHeight () ) ;
          int bot = int( abox.min[1] + yy - legendFont.getStringDescender () ) ;
          if ( ( cpos > abox.min[0] ) && ( cpos < abox.max[0] ) )
          {
            glColor4fv ( colour [ PUCOL_MISC ] ) ;
            glBegin   ( GL_LINES ) ;
            glVertex2i ( dx + cpos    , dy + bot ) ;
            glVertex2i ( dx + cpos    , dy + top ) ;
            glVertex2i ( dx + cpos - 1, dy + bot ) ;
            glVertex2i ( dx + cpos - 1, dy + top ) ;
            glVertex2i ( dx + cpos - 4, dy + bot ) ;
            glVertex2i ( dx + cpos + 3, dy + bot ) ;
            glVertex2i ( dx + cpos - 4, dy + top ) ;
            glVertex2i ( dx + cpos + 3, dy + top ) ;
            glEnd      () ;
          }
        }

        val[ cursor_position ] = temp_char ;
      }
    }
  }

  draw_label ( dx, dy ) ;
}


int puLargeInput::checkHit ( int button, int updown, int x, int y )
{
  if ( bottom_slider )
  {
    if ( bottom_slider->checkHit ( button, updown, x, y ) ) return TRUE ;
  }

  if ( right_slider->checkHit ( button, updown, x, y ) ) return TRUE ;

  // If the user has clicked within the bottom slider or to its right, don't activate.

  if ( y < slider_width ) return FALSE ;

  if ( puObject::checkHit ( button, updown, x, y ) )
    return TRUE ;

  return FALSE ;
}


void puLargeInput::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    // Active widget exists and is not this one; call its down callback if it exists

    puActiveWidget()->invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    // Most GUI's activate a button on button-UP not button-DOWN.

    // Text and window parameters:

    int line_size = legendFont.getStringHeight () +         // Height of a line
                    legendFont.getStringDescender() + 1 ;  // of text, in pixels

    int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
//  int box_height = ( abox.max[1] - abox.min[1] ) / line_size ;
                                               // Input box height, in lines

    float bottom_value = bottom_slider ? bottom_slider->getFloatValue () : 0.0 ;

    int beg_pos      // Position in window of start of line, in pixels
                = int( ( box_width - max_width ) * bottom_value ) ;
//  int end_pos      // Position in window of end of line, in pixels
//              = (int)( beg_pos + max_width - 1 ) ;
    if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
//  int end_lin      // Position on line count of bottom of window, in lines
//              = top_line_in_window + box_height - 1 ;

//  int xx = legendFont.getStringWidth ( " " ) ;
    int yy = int( abox.max[1] - legendFont.getStringHeight () * 1.5 
            + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

    // Get the line number and position on the line of the mouse

    char *strval = bottom_slider ? getText () : getWrappedText () ;
    char *tmpval = new char [ strlen(strval) + 1 ] ;
    strcpy ( tmpval, strval ) ;

    int i = strlen ( tmpval ) ;

    char *end_of_line = strchr ( tmpval, '\n' ) ;
    char *start_of_line = tmpval;

    // Step down the lines until the y-coordinate is less than the mouse

    int line_count = 0 ;

    while ( ( yy > y ) && end_of_line )
    {
      line_count++ ;
      start_of_line = end_of_line + 1 ;
      yy -= line_size ;
      end_of_line = strchr ( start_of_line, '\n' ) ;
    }

    if ( end_of_line )
    {
      *end_of_line = '\0' ;

      i = strlen ( tmpval ) ;

      int length, prev_length = 0 ;
      while ( x <= (length = legendFont.getStringWidth ( start_of_line )
                    + abox.min[0] + beg_pos) &&
              i >= 0 )
      {
        prev_length = length ;
        tmpval[--i] = '\0' ;
      }

      if ( ( x - length ) < ( prev_length - x ) )
        i-- ;   // Mouse is closer to previous character than next character
    }

    // Now process the mouse click itself.

    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;

      accepting = TRUE ;
      cursor_position = i ;
      normalize_cursors () ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else if ( updown == PU_DOWN )
    {
      // We get here if the active edge is not down but the mouse button has
      // been pressed.  Start a selection.

      select_start_position = i ;
      select_end_position = i ;
    }
    else if ( updown == PU_DRAG )
    {
      // Drag -- extend the selection.

      if ( (select_end_position - i) > (i - select_start_position) )
        select_start_position = i ;   // Cursor closer to start than to end
      else
        select_end_position = i ;     // Cursor closer to end than to start

      if (select_start_position > select_end_position)
      {
        i = select_end_position ;
        select_end_position = select_start_position ;
        select_start_position = i ;
      }

    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}

int puLargeInput::checkKey ( int key, int /* updown */ )
{
  extern void puSetPasteBuffer ( char *ch ) ;
  extern char *puGetPasteBuffer () ;

  if ( !isAcceptingInput () || !isActive () || !isVisible () || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    // Active widget exists and is not this one; call its down callback if it exists

    puActiveWidget()->invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  normalize_cursors () ;

  //char *old_text = getText () ;
  char *old_text = bottom_slider ? getText () : getWrappedText () ;
  int lines_in_window = getLinesInWindow () ;  /* Added lines_in_window and num_lines to allow for "END" */
  int num_lines = getNumLines () ;             /* and PGUP/DOWN to work properly        - JCJ 20 Jun 2002 */
  int line_width = 0 ;                         /* Width of current line (for bottomslider) */
  int line_width_to_cursor = 0 ;               /* Width of current line up to the cursor position */
  int prev_line_width = 0 ;                    /* Width of previous line (for left mouse arrow) */
  int i = 1;                                   /* Happy useful variable */
  int line_counter = 0 ;                       /* Happy useful variable #2 */
  int box_width = abox.max[0] - abox.min[0] - slider_width ;   /* Text box width */
  int tmp_cursor_position = cursor_position ;  /* Temporary cursor position for counting without moving the cursor */
  int bottom_line_in_window = 0;                  /* # of bottom line in window */
  int current_line_in_window = 0;                 /* Current line # in window */
  char *line_end = 0 ;
  char* p = new char[ strlen(old_text)+1 ] ;
  float bottom_value = bottom_slider ? bottom_slider->getFloatValue () : 0.0 ; /* Value of the bottom slider */
  if (old_text[1] != '\0') /* Ensure that we don't delete something that doesn't exist! - JCJ 22 July 2002 */
  {
      /*Count how many characters from the beginning of the line the cursor is*/
      while ( ( old_text [ cursor_position - i ] != '\n' ) &&
               ( i < cursor_position ) )
       i++ ;            /* Step back to the beginning of the line */
      if ( i < cursor_position ) i-- ;

      /*Find the length of the current line*/
      while ( ( old_text [ tmp_cursor_position ] != '\n' ) &&
               ( tmp_cursor_position > 0) )
        tmp_cursor_position-- ;          
  
      p [0] = '\0' ;
      strcat ( p, ( old_text + tmp_cursor_position + 1 ) ) ;
      if ( bottom_slider )
      {
        line_end = strchr ( p, '\n' ) ;
        if ( line_end )  // Found an end-of-line
        {
            *line_end = '\0' ;  // Temporary break in line
            line_width = legendFont.getStringWidth ( p ) ;
            *line_end = '\n' ;  // Reset the carriage return
        }
        /*Now delete all characters in the string beyond i, and re-find its width*/
        p [i+1] = '\0' ;
        line_width_to_cursor = legendFont.getStringWidth ( p ) ;
      }

      /* Now find the length of the previous line */
      tmp_cursor_position-- ;          
      while ( ( old_text [ tmp_cursor_position ] != '\n' ) &&
              ( tmp_cursor_position > 0) )
        tmp_cursor_position-- ;          
      p [0] = '\0' ;
      strcat ( p, ( old_text + tmp_cursor_position + 1 ) ) ;
      if ( bottom_slider )
      {
        line_end = strchr ( p, '\n' ) ; 
        if ( line_end )  /*Actually, we KNOW there's a line after this one, but hey, let's be safe, eh? - JCJ 21 Jun 2002 */
        {
            *line_end = '\0' ;  // Temporary break in line
            prev_line_width = legendFont.getStringWidth ( p ) ;
            *line_end = '\n' ;  // Reset the carriage return
        }
      }

      delete [] p  ;
      p = NULL ;
  }

  bool done = true ;

  switch ( key )
  {
  case PU_KEY_PAGE_UP   :
    while ( old_text [ cursor_position ] != '\0' ) /* Move the cursor to the top of the data */
      cursor_position-- ;
    select_start_position = select_end_position = cursor_position ; 
    setTopLineInWindow ( ((top_line_in_window - lines_in_window + 2)<0) ? 0 : (top_line_in_window - lines_in_window + 2) );
    right_slider->setValue (1.0f - float(top_line_in_window) / num_lines ) ;
    break;
  case PU_KEY_PAGE_DOWN :
    while ( old_text [ cursor_position ] != '\0' ) /* Move the cursor to the end of the data */
      cursor_position++ ;
    select_start_position = select_end_position = cursor_position ; 
    setTopLineInWindow ( ((top_line_in_window + lines_in_window - 2)>num_lines) ? (num_lines + 2) : (top_line_in_window + lines_in_window - 2) ); /* Plus two for consistancy - JCJ 20 Jun 2002 */
    right_slider->setValue (1.0f - float(top_line_in_window) / num_lines ) ;
    break;
  case PU_KEY_INSERT    : return FALSE ;

  case PU_KEY_UP   :
  case PU_KEY_DOWN :
    /* Determine the current line, the line at the top of the window, and the line at the bottom.*/
    bottom_line_in_window = top_line_in_window + getLinesInWindow() ;
    while ( line_counter < cursor_position) {
      if ( old_text [ line_counter ] == '\n' ) current_line_in_window++ ;
      line_counter++ ;
    }
    if ( key == PU_KEY_UP )
    {
      // Step backwards to the beginning of the previous line
      cursor_position -= (i + 2) ;
      while ( ( old_text [ cursor_position ] != '\n' ) &&
              ( cursor_position > 0 ) )
        cursor_position-- ;
      if ( cursor_position > 0 ) cursor_position++ ;

      // Step down the line "i" spaces or to the end of the line
      while ( ( old_text [ cursor_position ] != '\n' ) &&
              ( i > 0 ) )
      {
        cursor_position++ ;
        i-- ;
      }
      if (current_line_in_window <= top_line_in_window) {
          setTopLineInWindow ( top_line_in_window - 1 ); /* Go up - JCJ 21 Jun 2002 */
          right_slider->setValue (1.0f - float(top_line_in_window) / num_lines ) ;
      }
    }
    else   // Down-arrow key
    {
      // Skip to beginning of next line
      while ( old_text [ cursor_position ] != '\n' && old_text [ cursor_position ] != '\0' )
        cursor_position++ ;
      cursor_position++ ;

      // Step down the line "i" spaces or to the end of the line
      // or to the end of the text

      while ( ( old_text [ cursor_position ] != '\n' ) &&
              ( cursor_position < (int)strlen ( old_text ) ) &&
              ( i > 0 ) )
      {
        cursor_position++ ;
        i-- ;
      }
      if ((current_line_in_window+1) >= bottom_line_in_window) {
          setTopLineInWindow ( top_line_in_window + 1 ); /* Go down - JCJ 21 Jun 2002 */
          right_slider->setValue (1.0f - float(top_line_in_window) / num_lines ) ;
      }
          
    }

    select_start_position = select_end_position = cursor_position ;
    break ;

  case 0x1B :  // ESC
  case '\t' :  // TAB  -- End of input
    rejectInput () ;
    normalize_cursors () ;
    invokeCallback () ;
    puDeactivateWidget () ;
    break ;

  case PU_KEY_HOME   :
    while ( ( old_text [ cursor_position-1 ] != '\n' )  &&
              ( cursor_position > 0 ) )               /* Move the cursor to the start of the line, but minus one so */
      cursor_position-- ;                             /* it does not find a \n immediately if located at the end.   */
    select_start_position = select_end_position = cursor_position ; /* Also corrects overshoots.  - JCJ 20 Jun 2002 */
    if ( bottom_slider ) bottom_slider->setValue ( 0.0f ) ;
    break ;
  case PU_KEY_END    :
    if (bottom_slider)
    {
      bottom_slider->setValue ( ( (line_width - box_width)<0 ) ? 0.0f : float(line_width+10) / max_width);
    }
    while ( old_text [ cursor_position ] != '\n' ) /* Move the cursor to the end of the line - JCJ 20 Jun 2002 */
      cursor_position++ ;
    select_start_position = select_end_position = cursor_position ;
    break ;
  case PU_KEY_LEFT   :
  case PU_KEY_RIGHT  :
    if ( key == PU_KEY_LEFT ) {
        cursor_position-- ; /* Left key pressed */
        if (old_text [ cursor_position ] == '\n')
            bottom_slider->setValue( ( (prev_line_width - box_width)<0 ) ? 0.0f : float(prev_line_width+10) / max_width);
        /* If the cursor is going off the left edge of the box, scroll left. */
        else if ((bottom_value*max_width) > line_width_to_cursor+5) {
            bottom_slider->setValue( ((bottom_value*max_width)-(box_width/2)-5)<0 ? 0.0f :
              ((bottom_value*max_width)-(box_width/2)-5)/max_width ) ;
        }
    } else {
        cursor_position++ ; /* Right key pressed */
        if (old_text [ cursor_position-1 ] == '\n')
          bottom_slider->setValue(0.0f) ;
        else if ((bottom_value*max_width)+(box_width) < line_width_to_cursor+5) {
          bottom_slider->setValue( ((bottom_value*max_width)+(box_width/2)+5)/max_width ) ;
        }
    }
    select_start_position = select_end_position = cursor_position ;
    break ;

  default :
    done = false ;
    break ;
  }

  if ( ! done && ! input_disabled )
  {
    char *p = NULL ;
    int temp_cursor = cursor_position ;

    switch ( key )
    {
    case '\b' :  // Backspace
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;
      else if ( cursor_position > 0 )
      {
        p = new char [ strlen(old_text) ] ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ --cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
        setText ( p ) ;
        setCursor ( temp_cursor - 1 ) ;
        delete [] p ;
      }

      break ;

    case 0x7F :  // DEL
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;
      else if (cursor_position != (int)strlen ( old_text ) )
      {
        p = new char [ strlen(old_text) ] ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
        setText ( p ) ;
        setCursor ( temp_cursor ) ;
        delete [] p ;
      }

      break ;

    case 0x15 /* ^U */ : getText () [ 0 ] = '\0' ; break ;
    case 0x03 /* ^C */ :
    case 0x18 /* ^X */ :  /* Cut or copy selected text */
      if ( select_start_position != select_end_position )
      {
        p = getText () ;
        char ch = p[select_end_position] ;
        p[select_end_position] = '\0' ;
        puSetPasteBuffer ( p + select_start_position ) ;
        p[select_end_position] = ch ;

        if ( key == 0x18 )  /* Cut, remove text from string */
          removeSelectRegion () ;
      }

      break ;

    case 0x16 /* ^V */ : /* Paste buffer into text */
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;

      p = puGetPasteBuffer () ;
      if ( p )  // Make sure something has been cut previously!
      {
        p = new char [ strlen ( getText () ) + strlen ( p ) + 1 ] ;
        strncpy ( p, getText (), cursor_position ) ;
        p[cursor_position] = '\0' ;
        strcat ( p, puGetPasteBuffer () ) ;
        strcat ( p, getText() + cursor_position ) ;
        temp_cursor += strlen ( puGetPasteBuffer () ) ;
        setText ( p ) ;
        setCursor ( temp_cursor ) ;
        delete [] p ;
      }

      break ;

    default:
      if ( ( key < ' ' || key > 127 ) && ( key != '\n' )
                                      && ( key != '\r' ) ) return FALSE ;

      if ( key == '\r' ) key = '\n' ;

      if ( valid_data )
      {
        if ( !strchr ( valid_data, key ) ) return TRUE ;
      }

      if ( select_start_position != select_end_position ) // remove selected text
      {
        temp_cursor -= ( select_end_position - select_start_position ) ;
        removeSelectRegion () ;
      }

      p = new char [ strlen(old_text) + 2 ] ;

      strncpy ( p, old_text, cursor_position ) ;

      p [ cursor_position ] = key ;
      p [ cursor_position + 1 ] = '\0' ;
     
      strcat (p, ( old_text + cursor_position ) ) ;
      bottom_line_in_window = top_line_in_window + getLinesInWindow() ;

      /* If running off the screen, scroll right. - JCJ 28 Jun 2002 */
      if ((bottom_value*max_width)+(box_width) < line_width_to_cursor+5) {
          bottom_slider->setValue( ((bottom_value*max_width)+(box_width/2)+5)/max_width ) ;
      }

      if (key == '\n') {
      /* If pressing enter, figure out which line this is. */
          while ( line_counter < cursor_position) {
              if ( old_text [ line_counter ] == '\n' ) current_line_in_window++ ;
              line_counter++ ;
          }
      /* If hitting enter at the bottom of the screen, scroll down. - JCJ 28 Jun 2002 */
          if ( (current_line_in_window+1) >= bottom_line_in_window ) {
              setTopLineInWindow ( top_line_in_window + 1 ); 
              right_slider->setValue (1.0f - float(top_line_in_window) / num_lines ) ;
          }
      }

      setText ( p ) ;
      setCursor ( temp_cursor + 1 ) ;
      delete [] p ;

      break ;
    }
  }

  normalize_cursors () ;
  return TRUE ;
}

void puLargeInput::wrapText ( void )
{
  // Wrap the text in "text" and put it in "wrapped_text"

  int l_len = strlen ( getStringValue () ) ;
  delete [] wrapped_text ;
  wrapped_text = new char[l_len + 1] ;
  memcpy(wrapped_text, getStringValue (), l_len + 1) ;

  char *wrapped_text_wp = wrapped_text,
       *space_ptr,
       *old_space_ptr ;

  /* Somewhat inspired by tuxracer */
  while ( *wrapped_text_wp != '\0' )
  {
    old_space_ptr = NULL ;
    space_ptr = strchr ( wrapped_text_wp, ' ' ) ;

    while (1)
    {
      if ( space_ptr != NULL )
        *space_ptr = '\0' ;

      if ( legendFont.getStringWidth ( wrapped_text_wp ) >
           (   ( abox.max[0] - abox.min[0] )
             - slider_width
             - PUSTR_LGAP
             - ( getStyle () == PUSTYLE_BOXED ? getBorderThickness () : 0 )
             - PUSTR_RGAP
           )
         )
        break ;

      old_space_ptr = space_ptr ;

      if ( space_ptr == NULL )
        /* Entire string fits in widget */
        break ;

      // Check for carriage return in the original string
      if ( strrchr ( wrapped_text_wp, '\n' ) > wrapped_text_wp )
        wrapped_text_wp = strrchr ( wrapped_text_wp, '\n' ) + 1 ;

      *space_ptr = ' ' ;

      space_ptr = strchr ( space_ptr+1, ' ' ) ;
    }

    if ( old_space_ptr == NULL )
    /* Either string is too wide for area, or the entire remaining portion
       of string fits in area (space_ptr == NULL). */
    {
      wrapped_text_wp += strlen (wrapped_text_wp) ;

      if ( space_ptr != NULL )
      /* Advance past the NULL since there's more string left */
        wrapped_text_wp += 1 ;
    }
    else
    {
      if ( space_ptr != NULL )
        *space_ptr = ' ' ;
      *old_space_ptr = '\n' ;

      wrapped_text_wp = old_space_ptr + 1 ;
    }
  }
}


