# Test's / Analysis
The original file "test.file" (md5sum: **64ce7d40f01370bdda17bdf0f7b01b56**) contains random data 
and has been transfered from linux mint's ftp client to android "Wifi Pro FTP Server" without any definition if ascii or binary mode should be used.
The file "test_ftptransferred.file" (md5sum: **ab01a95535172826b99a818d3e2e600e**) got mangled during transfer.

```
cmp -l test.file test_ftptransferred.file | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}' > cmp_binary_comparison.txt
```
a binary file comparison shows that 0x0d got replaced by 0x0a (see cmp_binary_comparison.txt).

# Conclusion
We have to find every 0x0a in the broken file and -as we don't know which 0x0a had been there before in the originating file- must try to replace each case with 0x0d for every possible combination. Every combination must then be verified using zlib (the gzip library).