#ifndef ERROR_DIALOG_DARWIN_H
#define ERROR_DIALOG_DARWIN_H
/*
 *  ErrorDialog_darwin.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Wed Jul 23 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "ErrorDialog.h"

class ErrorDialog_darwin : public ErrorDialog
{
public:
    void ShowError();
};

#undef ARCH_ERROR_DIALOG
#define ARCH_ERROR_DIALOG ErrorDialog_darwin 

#endif /* ERROR_DIALOG_DARWIN_H */
