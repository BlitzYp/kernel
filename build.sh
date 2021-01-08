g++ -c main.cc -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
ld -T linker.ld -o kernel.bin boot.o kernel.o
echo "Built, you can now run with QEMU"
