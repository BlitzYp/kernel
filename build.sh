g++ -c src/main.cc -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
nasm -f elf64 src/boot.s -o boot.o
ld -T src/linker.ld -o kernel.bin boot.o kernel.o
echo "Built, you can now run with QEMU"
