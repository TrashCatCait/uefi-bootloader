include ./Makefile.var

CC=gcc
CFLAGS=-nostdlib -maccumulate-outgoing-args -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone   
CCINC=-I./includes/ -I./gnu-efi-code/inc/ -I./gnu-efi-code/inc/protocol/ -I./gnu-efi-code/inc/x86_64/
##Linker settings
LD=ld
LFLAGS=-shared -nostdlib -Bsymbolic -L./gnu-efi-code/x86_64/gnuefi -L./gnu-efi-code/x86_64/lib -T./gnu-efi-code/gnuefi/elf_x86_64_efi.lds 
LIBS=-lefi -lgnuefi
##objcopy settings
OBJC=objcopy 
OBJCFLAGS=-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10
BUILD=build

SRCFILES=$(wildcard ./src/*.c)
OBJFILES=$(patsubst ./src/%.c, $(BUILD)/%.o, $(SRCFILES))

$(EFIBOOT): $(BUILD) ./$(BUILD)/main.so 
	$(OBJC) $(OBJCFLAGS) ./build/main.so $@

./$(BUILD)/main.so: $(OBJFILES) 
	$(LD) $(LFLAGS) ./gnu-efi-code/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o $@ -lgnuefi -lefi

./$(BUILD)/%.o: ./src/%.c
	$(CC) $(CCINC) $(CFLAGS) -c $^ -o $@

$(BUILD): 
	mkdir $@ 

.PHONY: clean 

clean: 
	rm -rf $(BUILD) $(EFIBOOT)
