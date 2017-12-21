Explanation                             {#explanation}
============
_so called "systemd" team_

Input / Output
--------------
The First features that we implement are inputs and output for the user.  
The user calls functions that are provided in syscall.h and trigger a system call.  
The code of the system call is put in register r2 and then a kernel trap is triggered.  
The kernel trap is handeled by a call to the MIPS command syscall which triggers an  
exception handler in kernel mode. The exception handler will identify the system call  
based on the content of r2 and will lunch the proper function.  

### GetChar() ###

### PutChar() ###

### GetString() ###

### GetString() ###

### GetInt() ###

### PutInt() ###
<br/>
code/userprog/synchconsole.h  
code/userprog/synchconsole.cc


Threads
-------

