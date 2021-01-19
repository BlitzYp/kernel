# nasm -f elf32 src/boot.s -o boot.o
yasm -p gas -f elf32 src/boot.s -o boot.o 
clang++ -m32 -c src/main.cc -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
ld -m elf_i386 -T src/linker.ld -o kernel.bin boot.o kernel.o
echo "Built kernel.bin."
if test "$1" = "iso"; then
    echo "Building kernel.iso, please wait..."
    mkdir -p iso/boot/grub
    cp kernel.bin iso/boot 
    cp src/grub.cfg iso/boot/grub
    grub-mkrescue -o kernel.iso iso/
    echo "Built kernel.iso."
fi
