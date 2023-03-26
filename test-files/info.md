# Test's / Analysis
The original file "test.file.gz" (md5sum: **73d38b1bf5f903f5d976788ea54939c3**) contains random data 
and has been transfered from linux mint's ftp client to android "Wifi Pro FTP Server" without any definition if ascii or binary mode should be used.
The file "test_corrupted.file.gz" (md5sum: **cdfc14d29a1d9b5c5049a4f97ac8a0e6**) got mangled during transfer.

```
cmp -l test.file.gz test_corrupted.file.gz | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}' > cmp_binary_comparison.txt
```
a binary file comparison shows that 0x0D got replaced by 0x0A (see cmp_binary_comparison.txt).

# Conclusion
We have to find every 0x0A in the broken file and -as we don't know which 0x0A had been there before in the originating file- must try to replace each case with 0x0d for every possible combination. Every combination must then be verified using zlib (the gzip library).