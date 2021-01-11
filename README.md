# kernel

tiny i686 (soon to be x86_64) kernel in C++ and Assembly, as a submission for PD hackathon... also may be useful reference when writing your own kernel. licensed under the quite liberal BSD 3-clause license; check out ~~LICENSE.md~~ sorry i forgot here it is https://opensource.org/licenses/BSD-3-Clause

requirements for compilation:
- An assembler compatible with AT&T (specifically gas) syntax that can compile a freestanding i686 elf binary
    * should work fine on YASM or gas. 
    * recommend i686-elf-{as,gcc} or yasm with gas syntax support. we use the former
- A C++ compiler
    * G++ works great
    * clang++ does weird shit


