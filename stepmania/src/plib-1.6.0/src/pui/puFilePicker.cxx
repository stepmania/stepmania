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

/****
* NAME
*   puFilePicker
*
* DESCRIPTION
*   PUI dialog for picking files
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   August 2000
*
* MODIFICATION HISTORY
*   John Fay - many improvements
*   Steve Baker - Added functionality to allow directory tree traversal
****/


#include "puLocal.h"

UL_RTTI_DEF1(puFilePicker,puDialogBox)


#if defined(WIN32) && !defined(__CYGWIN__)
#define DOTDOTSLASH "..\\"
#define SLASH       "\\"
#else
#define DOTDOTSLASH "../"
#define SLASH       "/"
#endif

static void puFilePickerHandleSlider ( puObject * slider )
{
  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;

  puListBox* list_box = (puListBox*) slider -> getUserData () ;
  int idx = int ( list_box -> getNumItems () * val ) ;
  list_box -> setTopItem ( idx ) ;
}

static void puFilePickerHandleArrow ( puObject *arrow )
{
  puSlider *slider = (puSlider *) arrow->getUserData () ;
  puListBox* list_box = (puListBox*) slider -> getUserData () ;

  int type = ((puArrowButton *)arrow)->getArrowType() ;
  int inc = ( type == PUARROW_DOWN     ) ?   1 :
            ( type == PUARROW_UP       ) ?  -1 :
            ( type == PUARROW_FASTDOWN ) ?  10 :
            ( type == PUARROW_FASTUP   ) ? -10 : 0 ;

  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;
  int num_items = list_box->getNumItems () - 1 ;
  if ( num_items > 0 )
  {
    int idx = int ( num_items * val + 0.5 ) + inc ;
    if ( idx > num_items ) idx = num_items ;
    if ( idx < 0 ) idx = 0 ;

    slider -> setValue ( 1.0f - (float)idx / num_items ) ;
    list_box -> setTopItem ( idx ) ;
  }
}


static void chop_file ( char *fname )
{
  /* removes everything back to the last '/' */

  for ( int i = strlen(fname)-1 ; fname[i] != SLASH[0] && i >= 0 ; i-- )
    fname[i] = '\0' ;
}


static void go_up_one_directory ( char *fname )
{
  /* removes everything back to the last but one '/' */

  chop_file ( fname ) ;

  if ( fname [ 0 ] == '\0' )
  {
    /* Empty string!  The only way to go up is to append a "../" */

    strcpy ( fname, DOTDOTSLASH ) ;
    return ;
  }

  /* If the last path element is a "../" then we'll have to add another "../" */

  if ( strcmp ( & fname [ strlen(fname)-3 ], DOTDOTSLASH ) == 0 )
  {
    if ( strlen ( fname ) + 4 >= PUSTRING_MAX )
    {
      ulSetError ( UL_WARNING, "PUI: puFilePicker - path is too long, max is %d.",
                          PUSTRING_MAX ) ;
      return ;
    }

    strcat ( fname, DOTDOTSLASH ) ;
    return ;
  }

  /* Otherwise, just delete the last element of the path. */

  /* Remove the trailing slash - then remove the rest as if it was a file name */

  fname [ strlen(fname)-1 ] = '\0' ;
  chop_file ( fname ) ;
}


void puFilePicker::handle_select ( puObject* l_box )
{
  puFilePicker* file_picker = (puFilePicker*) l_box -> getUserData () ;

  int selected ;
  l_box -> getValue ( &selected ) ;

  if ( selected >= 0 && selected < file_picker -> num_files )
  {
    char *dst = file_picker -> getStringValue () ;
    char *src = file_picker -> files [ selected ] ;

    chop_file ( dst ) ;

    if ( strcmp ( src, "[.]" ) == 0 )
    {
      /* Do nothing - but better refresh anyway. */

      file_picker -> find_files () ;
      return ;
    } 

    if ( strcmp ( src, "[..]" ) == 0 )
    {
      /* Do back up one level - so refresh. */

      go_up_one_directory ( dst ) ;
      file_picker -> find_files () ;
      return ;
    } 

    if ( file_picker -> dflag [ selected ] )
    {
      /* If this is a directory - then descend into it and refresh */

      if ( strlen ( dst ) + strlen ( src ) + 2 >= PUSTRING_MAX )
      {
	ulSetError ( UL_WARNING,
          "PUI: puFilePicker - path is too long, max is %d.", PUSTRING_MAX ) ;
	return ;
      }

      strcat ( dst, &src[1] ) ;  /* Remove leading '[' */
      dst [ strlen ( dst ) - 1 ] = SLASH[0] ;  /* Replace trailing ']' with slash */
      file_picker -> find_files () ;
      return ;
    }

    /* If this is a regular file - then just append it to the string */

    if ( strlen ( dst ) + strlen ( src ) + 2 >= PUSTRING_MAX )
    {
      ulSetError ( UL_WARNING, 
        "PUI: puFilePicker - path is too long, max is %d.", PUSTRING_MAX ) ;
      return ;
    }

    strcat ( dst, src ) ;
  }
  else
  {
    /*
      The user clicked on blank screen - maybe we should
      refresh just in case some other process created the
      file.
    */

    file_picker -> find_files () ;
  }
}


static void puFilePickerHandleCancel ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;
  file_picker -> setValue ( "" ) ;
  file_picker -> invokeCallback () ;
}

static void puFilePickerHandleOk ( puObject* b )
{
  puFilePicker* file_picker = (puFilePicker*) b -> getUserData () ;

  file_picker -> invokeCallback () ;
}

void puFilePicker::setSize ( int w, int h )
{
  // Resize the frame widget
  frame->setSize ( w, h ) ;

  // Resize and position the slider
  slider->setPosition ( w-30, 40+20*arrow_count ) ;
  slider->setSize ( 20, h-70-40*arrow_count ) ;

  // Position the arrow buttons
  if ( up_arrow )
  {
    up_arrow->setPosition ( w-30, h-30-20*arrow_count ) ;
    down_arrow->setPosition ( w-30, 20+20*arrow_count ) ;
  }

  if ( fastup_arrow )
  {
    fastup_arrow->setPosition ( w-30, h-50 ) ;
    fastdown_arrow->setPosition ( w-30, 40 ) ;
  }

  // Resize the list box
  list_box->setSize ( w-40, h-70 ) ;

  // Resoze and position the buttons
  cancel_button->setSize ( (w<170)?(w/2-15):70, 20 ) ;
  ok_button->setSize ( (w<170)?(w/2-15):70, 20 ) ;
  ok_button->setPosition ( (w<170)?(w/2+5):90, 10 ) ;
}

puFilePicker::~puFilePicker ()
{
  if ( files )
  {
    for ( int i=0; i<num_files; i++ )
      delete [] files[i];

    delete[] files;
    delete[] dflag;
  }
}

void puFilePicker::puFilePickerInit ( int x, int y, int w, int h, int arrows,
                                      const char *dir, const char *title )
{
  type |= PUCLASS_FILEPICKER ;

  files = NULL ;
  dflag = NULL ;
  num_files = 0 ;

  strcpy ( startDir, dir ) ;

  if ( arrows > 2 ) arrows = 2 ;
  if ( arrows < 0 ) arrows = 0 ;
  arrow_count = arrows ;

  frame = new puFrame ( 0, 0, w, h );

  slider = new puSlider (w-30,40+20*arrows,h-70-40*arrows,TRUE,20);
  slider->setDelta(0.1f);
  slider->setValue(1.0f);
  slider->setSliderFraction (0.2f) ;
  slider->setCBMode( PUSLIDER_DELTA );
  
  list_box = new puListBox ( 10, 40, w-40, h-30 ) ;
  list_box -> setLabel ( title );
  list_box -> setLabelPlace ( PUPLACE_TOP_LEFT ) ;
  list_box -> setStyle ( -PUSTYLE_SMALL_SHADED ) ;
  list_box -> setUserData ( this ) ;
  list_box -> setCallback ( handle_select ) ;
  list_box -> setValue ( 0 ) ;

  find_files () ;

  handle_select ( list_box ) ;

  slider -> setUserData ( list_box ) ;
  slider -> setCallback ( puFilePickerHandleSlider ) ;

  cancel_button = new puOneShot ( 10, 10, (w<170)?(w/2-5):80, 30 ) ;
  cancel_button -> setLegend ( "Cancel" ) ;
  cancel_button -> setUserData ( this ) ;
  cancel_button -> setCallback ( puFilePickerHandleCancel ) ;

  ok_button = new puOneShot ( (w<170)?(w/2+5):90, 10, (w<170)?(w-10):160, 30 ) ;
  ok_button -> setLegend ( "Ok" ) ;
  ok_button -> setUserData ( this ) ;
  ok_button -> setCallback ( puFilePickerHandleOk ) ;
//  ok_button->makeReturnDefault ( TRUE ) ;

  up_arrow = (puArrowButton *)NULL ;  down_arrow = (puArrowButton *)NULL ;
  fastup_arrow = (puArrowButton *)NULL ;  fastdown_arrow = (puArrowButton *)NULL ;

  if ( arrows > 0 )
  {
    down_arrow = new puArrowButton ( w-30, 20+20*arrows, w-10, 40+20*arrows, PUARROW_DOWN ) ;
    down_arrow->setUserData ( slider ) ;
    down_arrow->setCallback ( puFilePickerHandleArrow ) ;

    up_arrow = new puArrowButton ( w-30, h-30-20*arrows, w-10, h-10-20*arrows, PUARROW_UP ) ;
    up_arrow->setUserData ( slider ) ;
    up_arrow->setCallback ( puFilePickerHandleArrow ) ;
  }

  if ( arrows == 2 )
  {
    fastdown_arrow = new puArrowButton ( w-30, 40, w-10, 60, PUARROW_FASTDOWN ) ;
    fastdown_arrow->setUserData ( slider ) ;
    fastdown_arrow->setCallback ( puFilePickerHandleArrow ) ;

    fastup_arrow = new puArrowButton ( w-30, h-50, w-10, h-30, PUARROW_FASTUP ) ;
    fastup_arrow->setUserData ( slider ) ;
    fastup_arrow->setCallback ( puFilePickerHandleArrow ) ;
  }

  close  () ;
  reveal () ;
}

static int puFilePickerStringCompare ( const char *s1, const char *s2,
                                       const char  f1, const char  f2 )
{
  if ( f1 > f2 )    /* Directories before regular files. */
    return -1 ;

  if ( f1 < f2 )
    return 1 ;

  while ( 1 )
  {
    char c1 = s1? (*s1++): 0 ;
    char c2 = s2? (*s2++): 0 ;
    
    //end of string?
    if ( !c1 || !c2 )
    {
      if ( c1 )
        return 1 ; //s1 is longer
      if ( c2 )
        return -1 ; //s1 is shorter
      return 0 ;
    }
    
    if ( c1 == c2 )
      continue ;

    /*
      Windoze users are case-insensitive - so they presumably
      want case ignored in the sorting.

      UNIX users, however, do things like calling their makefiles
      'Makefile' so as to force them to the start of the directory
      listing.
    */

#if defined(WIN32)
    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;
    
    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;
#endif
    
    if ( c1 != c2 )
    {
      if ( c1 < c2 )
        return -1 ;
      return 1 ;
    }
  }
  return 0 ;
}

static void puFilePickerSort ( char** list, char *flags, int size )
//
//  comb sort - a modified bubble sort
//    taken from BYTE, April 1991, ppg 315-320
//
{
  int switches ;
  int gap = size ;

  do
  {
    gap = ((gap * 197) >> 8);  // gap /= 1.3;

    switch (gap)
    {
      case 0:  // the smallest gap is 1 -- bubble sort
	gap = 1;
	break;

      case 9:  // this is what makes this Combsort11
      case 10:
	gap = 11;
	break;
    }

    switches = 0 ; // dirty pass flag

    int top = size - gap;

    for ( int i=0; i<top; ++i )
    {
      int j=i+gap;

      if ( puFilePickerStringCompare ( list [i], list [j],
                                       flags[i], flags[j] ) > 0 )
      {
        char* temp = list[i] ;
        list[i] = list[j] ;
        list[j] = temp ;

        char tmp2 = flags[i] ;
        flags[i] = flags[j] ;
        flags[j] = tmp2 ;

        ++switches;
      }
    }
  } while ( switches || gap > 1 ) ;
}


void puFilePicker::find_files ( void )
{
  if ( files != NULL )
  {
    for ( int i = 0 ; i < num_files ; i++ )
      delete [] files[i] ;

    delete [] files ;
    delete [] dflag ;
  }

  num_files = 0 ;

  char dir [ PUSTRING_MAX * 2 ] ;

  strcpy ( dir, startDir ) ;
  strcat ( dir, SLASH ) ;
  strcat ( dir, getStringValue() ) ;

  if ( dir [ strlen ( dir ) - 1 ] != SLASH[0] )  /* Someone forgot a trailing '/' */
    strcat ( dir, SLASH ) ;

  int ifile = 0 ;

  ulDir    *dirp = ulOpenDir ( dir ) ;
  ulDirEnt *dp ;

  if ( dirp == NULL )
  {
    perror ("puFilePicker") ;
    ulSetError ( UL_WARNING, "PUI:puFilePicker - can't open directory '%s'", dir ) ;
    num_files = 0 ;
    return ;
  }

  while ( ( dp = ulReadDir(dirp) ) != NULL )
    ifile++ ;

  ulCloseDir ( dirp ) ;

  num_files = ifile ;

  if ( num_files == 0 )
  {
    ulSetError ( UL_WARNING,
		   "PUI:puFilePicker - no entries in directory '%s'?!", dir ) ;
    num_files = 0 ;
    return ;
  }

  files = new char* [ num_files+1 ] ;
  dflag = new char  [ num_files+1 ] ;

  dirp = ulOpenDir ( dir ) ;

  if ( dirp == NULL )
  {
    perror ("puFilePicker") ;
    ulSetError ( UL_WARNING,
                   "PUI:puFilePicker - can't re-open directory '%s'", dir ) ;
    num_files = 0 ;
    return ;
  }

  for ( ifile = 0 ; (dp = ulReadDir(dirp)) != NULL && ifile < num_files ; ifile++ )
  {
    dflag[ ifile ] = dp->d_isdir ;

    if ( dflag[ ifile ] )
    {
      files[ ifile ] = new char[ strlen(dp->d_name)+4 ] ;
      strcpy ( files [ ifile ], "[" ) ;
      strcat ( files [ ifile ], dp->d_name ) ;
      strcat ( files [ ifile ], "]" ) ;
    }
    else
    {
      files[ ifile ] = new char[ strlen(dp->d_name)+1 ] ;
      strcpy ( files [ ifile ], dp->d_name ) ;
    }
  }

  files [ ifile ] = NULL ;

  ulCloseDir ( dirp ) ;

  puFilePickerSort( files, dflag, num_files ) ;

  list_box -> newList ( files ) ;
}

