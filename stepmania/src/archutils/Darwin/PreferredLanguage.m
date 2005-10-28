#import "Cocoa/Cocoa.h"

char *GetPreferredLanguage()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [def objectForKey:@"AppleLanguages"];
	const char *lang = [[languages objectAtIndex:0] UTF8String];
	char *ret = (char *)malloc( strlen(lang) + 1 );

	strcpy(ret, lang);
    [pool release];
	return ret;
}	
