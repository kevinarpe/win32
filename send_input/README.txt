How to install Wine on Debian: https://wiki.debian.org/Wine

Packages installed on Debian stable (bullseye/11)
* fonts-wine:all
* libwine:amd64
* wine64:amd64
* wine:all
* libwine-dev:amd64
* wine64-tools:amd64
* mingw-w64:all
* wine32:i386
* winetricks:all

Then run: $ winetricks allfonts

How to display Japanese characters in a Linux Wine message box text?
https://stackoverflow.com/questions/71114503/how-to-display-japanese-characters-in-a-linux-wine-message-box-text

How to properly debug a cross-compiled Windows code on linux?
https://stackoverflow.com/a/68268087/257299

To debug:
term1$ wine Z:/usr/share/win64/gdbserver.exe localhost:12345 win32_wstr_test.exe
term2$ x86_64-w64-mingw32-gdb -ex 'set print elements 0' -ex 'target extended-remote localhost:12345' -ex 'b win32_wstr.c:532' -ex 'c' win32_wstr_test.exe
(gdb) target extended-remote localhost:12345
(gdb) b win32_wstr.c:213
(gdb) c
...
(gdb) kill
term1$ <Ctrl+C>, then re-run
term2$
(gdb) target extended-remote localhost:12345
(gdb) c
...
