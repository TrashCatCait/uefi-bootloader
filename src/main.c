#include <efi/efi.h>
#include <efi/efilib.h>
#include <elf.h> 
#include "../includes/efi-loader.h"

//Remember uefi call wrapper layout
//uefi_call_wrapper(function, argc, args)

typedef unsigned long long size_t; // 64 bit unsigned number.
const static CHAR16* welcomeMsg = L"Cait's EFI system loader \r\nVersion: %d.%d.%c\r\n";

EFI_FILE* load_kernel(){
    //HipptyHoopity Todo: 
    return NULL;
}

//This will be moved into it's own file. Basically the idea is to produce cleaner looking code.
void clr_scr()
{
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
}


EFI_STATUS efi_main (EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) 
{
    //InitializeLib efi libary system table and image handle 
    InitializeLib(imageHandle, systemTable);
    //Clear the screen of current contents. As it could still have left overs from the firmware manaufacter such as the motherboard logo.
    clr_scr();
    Print(welcomeMsg, EFILOADER_VER_MAJ, EFILOADER_VER_MIN, EFILOADER_STABLE);
    
    //Load Kernel File into memory from system ESP partion
    EFI_FILE* kerImg = NULL;
    //Check our kernel is valid
    if(kerImg == NULL) Print(L"Unable to load kernel image\r\n");
    else Print(L"Kernel successfully loaded\r\n");
    

    //Get computer memory map for use later
    while(1);
    return EFI_SUCCESS;
}

/*
 *
//I spent ages on this just to find it was frezzing because I forgot to use uefi_call_wrapper
EFI_FILE *LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_FILE* LoadedFile;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;

    uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);
    uefi_call_wrapper(ST->BootServices->HandleProtocol, 3, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem); 
    
    if (Directory == NULL) uefi_call_wrapper(FileSystem->OpenVolume, 2, FileSystem, &Directory);
    
    EFI_STATUS status = uefi_call_wrapper(Directory->Open, 5, Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    //EFI failed if yes return NULL
    if(status != EFI_SUCCESS) return NULL;

    return LoadedFile;
}


int memcmp(const void* aptr, const void* bptr, size_t n){
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

typedef struct {
    EFI_MEMORY_DESCRIPTOR* memMap;
    UINTN memMapSize;
    UINTN memMapDescSize;
} MemoryMap;
 *
 * EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    
    //Clearing is done to just ensure logos from firmware or mobo mana are no longer displayed
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    //Attempt to load the file kernel.elf from the / direcotry 
    EFI_FILE* KerImg = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
    //If kernel load fails print a message to inform the user of the failure
    if(KerImg == NULL) uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Unable to load Kernel\r\n");
    else uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Kernel Loaded\r\n");

    //Define our file header for our kernel  
    Elf64_Ehdr header;
    {
	UINTN FileInfoSize;
	EFI_FILE_INFO* FileInfo;
	uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, FileInfoSize, (void**)&FileInfo);
	uefi_call_wrapper(KerImg->GetInfo, 4, KerImg, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

	UINTN size = sizeof(header);
	uefi_call_wrapper(KerImg->Read, 3, KerImg, &size, &header);
    }
    
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Attempting to give control to Kernel\r\n");
    
    //Check the file header of the loaded file. Inform the user if it's valid or not.
    if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC || header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Kernel Header Elf Format Invalid\r\n");
    else uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Kernel Header Elf Format Verified\r\n");
    
    
    Elf64_Phdr* phdrs;
    {
	uefi_call_wrapper(KerImg->SetPosition, 2, KerImg, header.e_phoff);
	UINTN size = header.e_phnum * header.e_phentsize;
	uefi_call_wrapper(ST->BootServices->AllocatePages, 3, EfiLoaderData, size, (void**)&phdrs);
	uefi_call_wrapper(KerImg->Read, 3, KerImg, &size, phdrs);
    }
    
    //For each page hdr AllocatePages for the kernel
    for(Elf64_Phdr* phdr = phdrs; (char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize; phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize))
    {
	switch (phdr->p_type){
	    case PT_LOAD:
	    {
		int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
		Elf64_Addr segment = phdr->p_paddr;
		uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &segment);
		uefi_call_wrapper(KerImg->SetPosition, 2, KerImg, phdr->p_offset);
		UINTN size = phdr->p_filesz;
		uefi_call_wrapper(KerImg->Read, 3, KerImg, &size, (void*)segment);
		break;
	    }
	}
    }
    
    //Define the kernel Start
    int(*KernelStart)() = ((__attribute__((sysv_abi)) int(*)() ) header.e_entry);
    
    
    //Call the kernel start
    KernelStart();
    
    //Shutdown the PC if we come back here as in theroy we should have successfully booted and we shouldn't want to return here.
    uefi_call_wrapper(ST->RuntimeServices->ResetSystem, 4,EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    return EFI_SUCCESS;
}*/
