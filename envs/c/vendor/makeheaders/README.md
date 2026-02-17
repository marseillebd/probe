# Makeheaders

It's a self-contained C program by the author of SQLite and Fossil that generates headers from C/C++ source files.

I've mirrored the source file and html documentation from this tag[^tag], just in case the original urls fail for some reason.
I also think it would be nice (for myself) to try porting this to another language to gain some understanding, and perhaps improve the tool.

I've also packaged it with a nix derivation.

[^tag]: a456dcbbac25366ccb02696505ea04e99dace78ceb81056f7fc91537d3b14cc4

## Known Bugs

- It fails to parse the classic defer shim that works in gcc, and in so doing loses track of if-directive nesting.
