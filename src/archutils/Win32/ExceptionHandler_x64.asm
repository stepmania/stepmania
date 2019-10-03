extrn ?MainExceptionHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z : PROC

.data

pStack db 8000h dup(?) ; Allocate a new stack

.code

; long __stdcall CrashHandler::ExceptionHandler( EXCEPTION_POINTERS *pExc );
?ExceptionHandler@CrashHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z PROC

push rsp
lea rsp, offset pStack + 7FE0h ; Reserve register parameter stack area for RCX, RDX, R8 and R9
call ?MainExceptionHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z
pop rsp
ret

?ExceptionHandler@CrashHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z ENDP

END
