//
//  Helper.h
//  stepmania
//
//  Created by Steve Checkoway on Mon Sep 08 2003.
//  Copyright (c) 2003 Steve Checkoway. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Helper : NSObject
{
    NSString *mPath;
}
- (id)   initWithPath:(NSString *)path;
- (void) dealloc;
- (void) startHelper:(id)caller;
@end

@interface NSOpenPanel (SCCanCreateDirectories)
- (void) setCanCreateDirectories:(BOOL)b;
@end
