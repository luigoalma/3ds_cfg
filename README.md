# 3ds_cfg

Open source implementation of `config` system module.\
With intent of being a documentation supplement, but also working as replacement.\
Also in mind trying to make result binary as small as possible.

## Is this ready to use?

As much as I tested it, use at your own risk.

## Building

Just run `make`.\
It will create a cxi file, for Luma3DS before version 12.0, you can extract `code.bin` and `exheader.bin` with `ctrtool`, or some other tool, to place it in `/luma/titles/0004013000001702/`, otherwise for Luma3DS 12.0+ you can copy the cxi `/luma/sysmodules/` and rename it to `0004013000001702.cxi`.\
This requires game patching to be enabled on luma config on Luma3DS less than 12.0, requires external module loading on versions newer or equal to 12.0.

## License

This code itself is under Unlicense. Read `LICENSE.txt`\
The folders `source/3ds` and `include/3ds` have source files taken from [ctrulib](https://github.com/devkitPro/libctru), with modifications for the purpose of this program.\
Copy of ctrulib's license in `LICENSE.ctrulib.txt`

## Modifications to ctrulib

Ctrulib changed to generate smaller code, slimmed down sources and headers for a quicker read, and not depend on std libraries.\
As well some changes to behavior on result throw.\
SVCs were also made inlinable by making static inline functions that use `__asm__` and explicit register variables instead.\
What's not part of the original ctrulib, as of time of this code release, is the spi and i2c related service handle code.\
PS RSA structure is also modified.\
IPC header has extra functions to make cmdbuf checks easier as a service.\
Additionally `__cplusplus` checks for `extern "C"` without using base `3ds.h`
