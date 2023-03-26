# afgs
## asciiftp_gzip_sanitizer
afgs is an attempt to repair broken gzip-files which had been damaged during an ftp-transfer in ASCII-mode.

## What is FTP-ASCII-mode and why it is problematic for binary files.
FTP-ASCII-mode tries to convert line ending from windows-style (CRLF) to unix/linux-style (LF) and vice versa. This introduces problems when transferring binary files in ASCII-mode, as the bytes 0x0D 0x0A are modified to 0x0D (windows to unix) or 0x0D is getting modified to 0x0D 0x0A (unix to windows). For binary files therefore Binary-Mode should be used, as this modifications break binary-files such as .gzip-archives.

## Why is it so difficult to repair?
If you have a file, that had been altered by the process described above, it is not possible to reliably tell which 0x0D and 0x0A had been there before and which is there due to the modification.

## What does this program
In my special case, i transfered a gzip-file from linux to an android-ftp-server app. during transfer every 0x0D bytes got replaced by 0x0A. I therefore needed a solution to fix this. However simply re-replacing all 0x0A with 0x0D wouldn't work, as there most likely where 0x0A bytes in the original file as well, which mustn't be replaced by 0x0D.

This software reads in the whole gzip-binary file into memory, maps all 0x0A-bytes and tries for each possible combination to undo possible modification by changing them back to 0x0D. Each candidate will then be tested against zlib if it is valid gzip-data.

If a valid combination is found, it will be written back to disk.

## How to compile
### Prerequisites
You'll need gcc/g++ -10 as this project uses c++17 functionality.
Furthermore you'll need zlib.

To compile simply use the Makefile

```
cd src
make
```

This should result in two files **corrupter** and **salvager**.
