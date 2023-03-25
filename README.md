# afgs
## asciiftp_gzip_sanitizer
afgs is an attempt to repair broken gzip-files which had been damaged during an ftp-transfer in ASCII-mode.

## What is FTP-ASCII-mode and why it is problematic for binary files.
FTP-ASCII-mode tries to convert line ending from windows-style (CRLF) to unix/linux-style (LF) and vice versa. This introduces problems when transferring binary files in ASCII-mode, as the bytes 0x0D 0x0A are modified to 0x0D (windows to unix) or 0x0D is getting modified to 0x0D 0x0A (unix to windows). For binary files therefore Binary-Mode should be used, as this modifications break binary-files such as .gzip-archives.

## Why is it so difficult to repair?
If you have a file, that had been altered by the process described above, it is not possible to reliably tell which 0x0D and 0x0A had been there before and which is there due to the modification.

## What does this program
This software maps all 0x0A (windows to linux) or all 0x0D 0x0A (linux to windows) in the file and tries for each possible combination to undo possible modification. Each candidate will then be tested using

```gzip -t```

