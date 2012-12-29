/** @brief ProductInfo - Branding strings. Don't forget to also change ProductInfo.inc! */

#ifndef PRODUCT_INFO_H
#define PRODUCT_INFO_H

/**
 * @brief A friendly string to refer to the product in crash dialogs, etc.
 *
 * As an example, use "StepMania" here, not "StepMania4".
 */
#define PRODUCT_FAMILY_BARE StepMania

/**
 * @brief A unique name for each application that you might want installed side-by-side with other applications.
 *
 * As an example, use "StepMania4" here, not "StepMania". 
 * It would cause a conflict with older versions such as StepMania 3.X.
 */
#define PRODUCT_ID_BARE StepMania 5

/**
 * @brief Version info displayed to the user.
 *
 * These are the 'official' version designations:
 * <ul>
 * <li>"v5.0 alpha #": Alpha versions (bug squashing, polishing until we reach beta)</li>
 * <li>"v5.0 beta #": Beta versions (bug squashing, _focus_ is on high priority bugs)</li>
 * <li>"v5.0 rc#": Release Candidates (if there are no problems, move on to final)</li>
 * <li>"v5.0.0": Final Releases</li>
 * </li></ul>
 */
#ifndef PRODUCT_VER_BARE
#define PRODUCT_VER_BARE v5.0 beta 1a
#endif

/**
 * @brief A unique ID for a build of an application.
 *
 * This is used in crash reports and in the network code's version handling.
 */
#define PRODUCT_ID_VER_BARE PRODUCT_FAMILY_BARE PRODUCT_VER_BARE

// These cannot be #undef'd so make them unlikely to conflict with anything
#define PRODUCT_STRINGIFY(x) #x
#define PRODUCT_XSTRINGIFY(x) PRODUCT_STRINGIFY(x)

#define PRODUCT_FAMILY		PRODUCT_XSTRINGIFY(PRODUCT_FAMILY_BARE)
#define PRODUCT_ID			PRODUCT_XSTRINGIFY(PRODUCT_ID_BARE)
#define PRODUCT_VER			PRODUCT_XSTRINGIFY(PRODUCT_VER_BARE)
#define PRODUCT_ID_VER		PRODUCT_XSTRINGIFY(PRODUCT_ID_VER_BARE)

#define VIDEO_TROUBLESHOOTING_URL "http://www.stepmania.com/stepmaniawiki.php?title=Video_Driver_Troubleshooting"
/** @brief The URL to report bugs on the program. */
#define REPORT_BUG_URL "http://ssc.ajworld.net/sm-ssc/bugtracker/"
#define SM_DOWNLOAD_URL	"http://code.google.com/p/sm-ssc/downloads/list"

#define CAN_INSTALL_PACKAGES true

#endif

/**
 * @file
 * @author Chris Danford (c) 2003-2005
 * @section LICENSE
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

