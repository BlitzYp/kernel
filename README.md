# kernel

tiny i686 (soon to be x86_64) kernel in C++ and Assembly, as a submission for PD hackathon... also may be useful reference when writing your own kernel. licensed under the quite liberal BSD 3-clause license; check out LICENSE.md

requirements for compilation:
- A POSIX-ish system with the below, plus sh and a `test` command
- An assembler compatible with AT&T (specifically gas) syntax that can compile a freestanding i686 elf binary
    * should work fine on YASM or gas. 
    * recommend i686-elf-{as,gcc} or yasm with gas syntax support. we use the former
- A C++ compiler
    * G++ and Clang++ should both do the trick.

to build run `sh build.sh`
to build an iso run `sh build.sh iso` (requires grub-mkrescue and xorriso)

if you wish to just test and don't want to go to the hassle of compiling, we offer prebuilt ones too:
a kernel.bin (standalone) or an iso (kernel.iso) can be found in the root directory.