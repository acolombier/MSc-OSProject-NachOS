Explanation                             {#explanation}
============
_so called "systemd" team_

Input / Output
--------------
The First features that we implement are inputs and output for the user. The user calls functions that are provided in syscall.h and trigger a system call. The code of the system call is put in register r2 and then a kernel trap is triggered. The kernel trap is handeled by a call to the MIPS command syscall which triggers an exception handler in kernel mode. The exception handler will identify the system callbased on the content of r2 and will launch the proper function.  

### int GetChar() ###
[GetChar](@ref SynchConsole::GetChar) reads a `char` in the standard input provided by the
user [console](@ref SynchConsole) and returns returns an `int` instead of `char` to deal
with the `EOF` problem.


### void PutChar(const char ch) ###
[PutChar](@ref SynchConsole::PutChar) writes a character `ch` in the [console](@ref SynchConsole).

### void GetString(char *s, int n) ###
[GetString](@ref SynchConsole::GetString) reads a string `s` of length `n` from the standard input
provided by the user [console](@ref SynchConsole) .   
We only read at most `n-1` characters therefore if there are less then `n-1` characters there will be a `\0` character somewhere in the string and if there are more than `n-1` characters, only `n-1` will be read and the last one will be set to `\0`.    
The size `n` is bound by [MAX_STRING_SIZE](@ref MAX_STRING_SIZE).

### void PutString(const char *s) ###
[PutString](@ref SynchConsole::PutString) writes the string `s` to the provided user [console](@ref SynchConsole).
The maximum possible length of the string is [MAX_STRING_SIZE](@ref MAX_STRING_SIZE).

### void GetInt(int *n) ###
[GetInt](@ref SynchConsole::GetInt) reads a string form the provided user [console](@ref SynchConsole) and converts it into an int. For now the convertion is done using the UNIX command `sscanf()` and will be modified. The reason we use `int*` is because internally the function is more similar to GetString() then to GetChar().

### void PutInt(int n) ###
[PutInt](@ref SynchConsole::PutInt) writes the integer `n` on the console. It converts the number into a string using the UNIX command snprint(). This will be changed in the future.

Threads
-------

### Locks ###
We implemented [kernel locks](@ref Locks) but the wrapper for the user to use them in user code is not implemented yet. For now we have a very unstable use of semaphores because we provided them to the user with a void pointer just to check that our work on threads works. Our locks work in kernel mode in order to do handle thread mnagement. From all this feature, we have the ability to do a Prod-Cons with at most [MAX_THREADS](@ref MAX_THREADS).

### Halt ###
Calling Halt() should not just instantly stop everything. We've decided to join all the threads and then close the system.
At the moment Exit() is mapped to Halt(). For this reason if there is no call to halt at the end of the main function, then the system calls the kernal halt, which does not make any safe stop, and this is actually leading to a bad internal error.

### int Join(tid_t t) ###
The function [join()](@ref Thread::join) used to join threads returns an `int` that can be [NULL_TID](@ref NULL_TID) = -1 (Thread not found : never existed or no longer running). This shows that we do check that the thread that should be joined exists.
