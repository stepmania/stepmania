#ifndef LOADING_WINDOW_MODULE_GTK
#define LOADING_WINDOW_MODULE_GTK

typedef const char *(*INIT)(int *argc, char ***argv);
typedef void (*SHUTDOWN)();
typedef void (*SETTEXT)( const char *s );

#endif
