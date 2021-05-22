##Compiler settings
CC=gcc
CFLAGS=-fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args  
CCINC=-I./includes/ -I./gnu-efi-code/inc/ -I./gnu-efi-code/inc/protocol/ -I./gnu-efi-code/inc/x86_64/
##Linker settings
LD=ld
LFLAGS=-shared -Bsymbolic -L./gnu-efi-code/x86_64/gnuefi -L./gnu-efi-code/x86_64/lib -T./gnu-efi-code/gnuefi/elf_x86_64_efi.lds 
LIBS=-lefi -lgnuefi
##objcopy settings
OBJC=objcopy 
OBJCFLAGS=-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 

main.iso: main.img
	mkdir iso
	cp $^ iso
	xorriso -as mkisofs -R -f -e $^ -no-emul-boot -o $@ iso

main.img: build ./bootx64.efi 
	dd if=/dev/zero of=$@ bs=1M count=200
	mkfs.vfat -F32 ./main.img
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ ./bootx64.efi ::EFI/BOOT
	mcopy -i $@ ./kernel.elf ::

./bootx64.efi: build ./build/main.so 
	$(OBJC) $(OBJCFLAGS) ./build/main.so $@

./build/main.so: ./build/main.o 
	$(LD) $(LFLAGS) ./gnu-efi-code/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o $@ -lgnuefi -lefi

./build/main.o: ./src/main.c
	$(CC) $(CCINC) $(CFLAGS) -c $^ -o $@

build: 
	mkdir build 

.PHONY: testiso

testiso: clean main.iso
	qemu-system-x86_64 -pflash ~/Documents/OvmfX64/DEBUG_GCC5/FV/OVMF.fd -cdrom main.iso

.PHONY: testimg

testimg: clean main.img
	qemu-system-x86_64 -pflash ~/Documents/OvmfX64/DEBUG_GCC5/FV/OVMF.fd -drive file=main.img

.PHONY: clean 

clean: 
	rm -rf build bootx64.efi iso main.img main.iso
