# Projects
* send_input: Map shortcut keys to send keys
  * https://github.com/kevinarpe/win32/tree/master/send_input

Debug notes:
term1$ wine Z:/usr/share/win64/gdbserver.exe --multi localhost:12345
term2$ binary=read_dir_changes.exe; x86_64-w64-mingw32-gdb -ex 'set print elements 0' -ex 'target extended-remote localhost:12345' -ex "set remote exec-file $binary" -ex 'b main.c:1605' -ex 'run' $binary

