/* ConvertString - Convert a string to UTF-8. */

#ifndef RAGEUTIL_CHAR_CONVERSIONS_H
#define RAGEUTIL_CHAR_CONVERSIONS_H

/* Convert a string to UTF-8 from the first possible encoding in the given comma-
 * separated list of encodings.  The only valid strings are "japanese" and "korean". 
 * Return true if the conversion was successful (or a no-op).  Return false and
 * leave the string unchanged if the conversion was unsuccessful. */
bool ConvertString( RString &str, const RString &encodings );

#endif
