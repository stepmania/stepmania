#ifndef AVUTIL_FFVERSION_H
#define AVUTIL_FFVERSION_H
/* Mac OS X hack to ensure the version is defined. */
#cmakedefine SM_FFMPEG_VERSION "${SM_FFMPEG_VERSION}"
#define FFMPEG_VERSION SM_FFMPEG_VERSION
#endif /* AVUTIL_FFVERSION_H */

