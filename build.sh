# nasm -f elf32 src/boot.s -o boot.o
i686-elf-as src/boot.s -o boot.o 
g++ -m32 -c src/main.cc -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti 
ld -m elf_i386 -T src/linker.ld -o kernel.bin boot.o kernel.o
echo "Built, you can now run with QEMU"
 
# 64 bit:
# nasm -f elf64 src/boot.s -o boot.o
# g++ -c src/main.cc -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
# ld -m elf_x86_64 -T src/linker.ld -o kernel.bin boot.o kernel.o
