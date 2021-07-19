#include "../gnu-efi-code/inc/efi.h"
#include "../gnu-efi-code/inc/efilib.h"
#include <elf.h> 

#include "../includes/efi-ver.h"

//Remember uefi call wrapper layout
//uefi_call_wrapper(function, argc, args)
//Casting to (CHAR16 *) isn't really needed but it fixes issues of warning showing up in my text editor.

typedef unsigned long long size_t; // 64 bit unsigned number.
const static CHAR16* welcomeMsg = (CHAR16 *)L"Cait's EFI system loader\r\nVersion: %d.%d.%c\r\n";

typedef struct {
    EFI_MEMORY_DESCRIPTOR *memMap;
    UINTN mapSize;
    UINTN descSize;
} memLay;

typedef struct {
    void* baseAdd;
    size_t bufferSize;
    UINT32 width;
    UINT32 height;
    UINT32 PPSL; // pixels per scan line
} frameBuffer;

typedef struct {
    frameBuffer frameBuf;
    memLay memLayout;
} bootInfo;

//Get volume to load files from. We use the imagehandle from efi main to get the ESP partition.
//This application was loaded from. Then we use libOpenRoot to open the root volume in a file handle
EFI_STATUS check_error(EFI_STATUS status, CHAR16 *action)
{
    if(status != EFI_SUCCESS)
    {
        Print((CHAR16 *)L"EFI call %s failed. With error %r", action, status);
        while(1){}
    }
    return EFI_SUCCESS;
}

EFI_FILE_HANDLE getVolume(EFI_HANDLE image){
    EFI_LOADED_IMAGE *loadedImage = NULL; // Loaded Image interface used for opening device
    EFI_GUID imageGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID; // Image GUID interface  
    EFI_STATUS status;
    //populate our loaded image
    status = uefi_call_wrapper(
            BS->HandleProtocol, 3, image, 
            &imageGUID, (void **) &loadedImage
            );
    check_error(status, (CHAR16 *)L"Failed to get volume handle imageGUID check");

    EFI_FILE_HANDLE loadedRoot; //loaded root partition volume 
    
    //Used this instead of IOVOLUME
    loadedRoot = LibOpenRoot(loadedImage->DeviceHandle);

    //Return our loaded root file
    return loadedRoot;
}

EFI_FILE_HANDLE open_kernel(EFI_FILE_HANDLE root, CHAR16* fileName){
    EFI_FILE_HANDLE kernelFileHandle;
    EFI_STATUS  status;
    //Open our file
    status = uefi_call_wrapper(root->Open, 5, root, &kernelFileHandle, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    check_error(status, (CHAR16 *) L"Failed to open kernel file handle");

    return kernelFileHandle;
}

//This is not very efficient but is a good way to just quickly and dirtyly get the kernel header.
//TODO: FIX THIS AND MAKE A MORE EFFICIENT KERNEL HEADER FUNCTION/*
Elf64_Ehdr kernel_header(EFI_FILE_HANDLE kerFileHandle, CHAR16* fileName) {
    Elf64_Ehdr header;
    UINT64 headerSize = sizeof(header);
    
    /*Get the files actual size
    fileInfo = LibFileInfo(kerFileHandle); //File information sturcuture. 
    fileSize = fileInfo->FileSize; //48644864
    FreePool(fileInfo); //Free it as we are done with it
    */
    AllocatePool(headerSize); //AllocatePool for reading our file into 
    
    EFI_STATUS status;
    
    status = uefi_call_wrapper(kerFileHandle->Read, 3, kerFileHandle, &headerSize, &header);
    check_error(status, (CHAR16 *) L"Failed to read in kernel file headers");
    return header; 
}

Elf64_Phdr *ker_pheader(Elf64_Ehdr header, EFI_FILE_HANDLE kerFileHandle) {
    Elf64_Phdr *pheader; 
    EFI_STATUS status;

    status = uefi_call_wrapper(kerFileHandle->SetPosition, 2, kerFileHandle, header.e_phoff);
    check_error(status, (CHAR16*)L"Failed to set kernel postion when getting p header");

    UINTN size = header.e_phnum * header.e_phentsize; //calulate overall size
    
    status = uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, size, (void **)&pheader);
    check_error(status, (CHAR16*) L"Failed to allocate pool for pheaders");

    status = uefi_call_wrapper(kerFileHandle->Read, 3, kerFileHandle, &size, pheader);
    check_error(status, (CHAR16 *) L"Failed to read kernel pheaders");

    return pheader;
}

//This will be moved into it's own file. Basically the idea is to produce cleaner looking code.
void clr_scr()
{
    EFI_STATUS status;
    status = uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    check_error(status, (CHAR16 *) L"Unable to clear screen");
}


EFI_STATUS efi_main (EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) 
{
    bootInfo kernelInfo;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    //InitializeLib efi libary system table and image handle 
    InitializeLib(imageHandle, systemTable);
    //status used to get the returned statuses of efi calls and check errors
    EFI_STATUS status;
    

    //clear the screen 
    clr_scr(); 
    Print(welcomeMsg, EFILOADER_VER_MAJ, EFILOADER_VER_MIN, EFILOADER_STABLE);
    Print((CHAR16*)L"%d\n\r", EFI_INVALID_PARAMETER);    
    //Open our ESP partions volume. 
    EFI_FILE_HANDLE rootPart = getVolume(imageHandle);
    
    //Load Kernel File into memory from system ESP partion
    EFI_FILE_HANDLE kerImg = open_kernel(rootPart, (CHAR16 *)L"kernel.elf");
    
    if(kerImg == NULL) Print((CHAR16 *)L"Unable to load kernel image\r\n");
    else Print((CHAR16 *)L"Kernel successfully loaded\r\n");
    
    //Get the kernel files header details
    Elf64_Ehdr header = kernel_header(kerImg, (CHAR16 *)L"kernel.elf");
    Print((CHAR16 *)L"%d\r\n", header.e_phoff);
    
    //Get the kernel files pheader details
    Elf64_Phdr *pheader = ker_pheader(header, kerImg);
    Print((CHAR16 *)L"PType: %lu, %llu\r\n", pheader->p_paddr, pheader->p_memsz);
    
    for (
		Elf64_Phdr* phdr = pheader;
		(char*)phdr < (char*)pheader + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type){
			case PT_LOAD:
			{
				Elf64_Addr segment = phdr->p_paddr;
				uefi_call_wrapper(kerImg->SetPosition, 2, kerImg, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				uefi_call_wrapper(kerImg->Read, 3, kerImg, &size, (void*)segment);
				break;
			}
		}
	}
    
    
    status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
    check_error(status, (CHAR16 *) L"Unable to locate GOP");
    
    kernelInfo.frameBuf.baseAdd = (void *)gop->Mode->FrameBufferBase;
    kernelInfo.frameBuf.bufferSize = gop->Mode->FrameBufferSize;
    kernelInfo.frameBuf.PPSL = gop->Mode->Info->PixelsPerScanLine;
    kernelInfo.frameBuf.height = gop->Mode->Info->VerticalResolution;
    kernelInfo.frameBuf.width = gop->Mode->Info->HorizontalResolution;

    //Define the kernel Start
    int(*KernelStart)(bootInfo) = ((__attribute__((sysv_abi)) int(*)() ) header.e_entry); 
    UINTN mapSize = 0, mapKey, descriptorSize;
    EFI_MEMORY_DESCRIPTOR *memoryMap;
    UINT32 descriptorVersion = 1;

    //some vars we will use to get memory map
    while(EFI_SUCCESS != (status = uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion))) {
        if(status == EFI_BUFFER_TOO_SMALL)
        {
            Print((CHAR16 *)L"Increasing size of efi memory map buffer\n\r");
            mapSize += 2 * descriptorSize;
            uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, mapSize, (void **)&memoryMap);
        } else {
            Print((CHAR16 *)L"Failed to set up efi memory map %d\n\r", status);
            while(1){}
        }   
    }
    kernelInfo.memLayout.memMap = memoryMap;
    kernelInfo.memLayout.mapSize = mapSize;
    kernelInfo.memLayout.descSize = descriptorSize;

    status = uefi_call_wrapper((void *)ST->BootServices->ExitBootServices, 2, imageHandle, mapKey);    
    check_error(status, (CHAR16 *) L"Error Exiting Boot services");
    //Call our kernel 
    KernelStart(kernelInfo);
    if(status == EFI_SUCCESS) {
        uefi_call_wrapper(RT->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
    
    return EFI_SUCCESS;
}
