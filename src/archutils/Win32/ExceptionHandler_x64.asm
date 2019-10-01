extrn ?MainExceptionHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z : PROC
extrn VirtualAlloc : PROC

.code

; long __stdcall CrashHandler::ExceptionHandler( EXCEPTION_POINTERS *pExc );
?ExceptionHandler@CrashHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z PROC

push rbp
mov rbp, rsp
push rsi
mov rsi, rcx
xor rcx, rcx ; lpAddress = nullptr
mov rdx, 8000h ; dwSize = iSize
mov r8d, 00003000h; flAllocationType = MEM_COMMIT | MEM_RESERVE
mov r9d, 04h ; flProtect = PAGE_READWRITE
call VirtualAlloc ; char *pStack = (char *) VirtualAlloc( nullptr, iSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
add rax, 8000h ; pStack += iSize;
mov rcx, rsi ; Restore pExc
pop rsi
; mov rsp, rax ; FIXME: This instruction causes MainExceptionHandler abortion.
call ?MainExceptionHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z
mov rsp, rbp
pop rbp
ret

?ExceptionHandler@CrashHandler@@YAJPEAU_EXCEPTION_POINTERS@@@Z ENDP

END
