#import "SMMainThread.h"
#import <stdarg.h>

@interface SMMainThread (SMMainThreadPrivate)
- (void) perform;
@end

@implementation SMMainThread (SMMainThreadPrivate)
- (void) perform
{
	[actions makeObjectsPerformSelector:@selector(invoke)];
}
@end

@implementation SMMainThread
- (id) init
{
	[super init];
	actions = [[NSMutableArray alloc] init];
	return self;
}

- (void) dealloc
{
	[self performOnMainThread];
	[actions release];
	[super dealloc];
}

- (void) addAction:(SEL)sel withTarget:(id)target
{
	[self addAction:sel withTarget:target andNumObjects:0];
}

- (void) addAction:(SEL)sel withTarget:(id)target andObject:(id)object
{
	[self addAction:sel withTarget:target andNumObjects:1, &object];
}

- (void) addAction:(SEL)sel withTarget:(id)target andNumObjects:(int)num, ...
{
	NSMethodSignature *sig = [target methodSignatureForSelector:sel];
	NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sig];
	int i;
	va_list ap;
	
	[invocation setTarget:target];
	[invocation setSelector:sel];
	va_start( ap, num );
	
	for( i = 0; i < num; ++i )
	{
		// The first two arguments are self and _cmd.
		[invocation setArgument:va_arg(ap, id) atIndex:i+2];
	}
	va_end( ap );
	[actions addObject:invocation];
}

- (void) performOnMainThread
{
	if( ![actions count] )
		return;
	/* This can be done with an NSInvocation without the need for - (void) perform
	 * but it just makes this complex and confusing. */
	[self performSelectorOnMainThread:@selector(perform) withObject:nil waitUntilDone:YES];
	[actions removeAllObjects];
}
@end

/*
 * (c) 2006 Steve Checkoway
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
