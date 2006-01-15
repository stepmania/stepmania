#import <Foundation/Foundation.h>

@interface SMMainThread : NSObject
{
	@private
	NSMutableArray *actions;
}
- (id) init;
- (void) dealloc;
- (void) addAction:(SEL)sel withTarget:(id)target;
- (void) addAction:(SEL)sel withTarget:(id)target andObject:(id)object;
- (void) addAction:(SEL)sel withTarget:(id)target andBool:(BOOL)b;
// You must pass pointers to the objects to this method.
- (void) addAction:(SEL)sel withTarget:(id)target andNumObjects:(int)num, ...;
- (void) performOnMainThread;
@end

#define ADD_ACTION0(mt, t, s)    [mt addAction:@selector(s) withTarget:(t)]
#define ADD_ACTION1(mt, t, s, o) [mt addAction:@selector(s) withTarget:(t) andObject:(o)]
#define ADD_ACTIONb(mt, t, s, b) [mt addAction:@selector(s) withTarget:(t) andBool:(b)]
#define ADD_ACTIONn(mt, t, s, n, ...) \
[mt addAction:@selector(s) withTarget:(t) andNumObjects:(n), ## __VA_ARGS__]

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
