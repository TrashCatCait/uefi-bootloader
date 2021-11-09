#include <efi.h>
#include <efilib.h>
#include <elf.h> 
#include <utils.h> 
#include <efi-ver.h>
#include <gop.h>
#include <rootfs.h>
#include <types.h>

UINT64 get_file_size(EFI_FILE_HANDLE fileHandle) {
    UINT64 ret;
    EFI_FILE_INFO *file_info;
    file_info = LibFileInfo(fileHandle);
    ret = file_info->FileSize;
    FreePool(file_info);
    return ret;
}


//Remember uefi call wrapper layout
//uefi_call_wrapper(function, argc, args)
EFI_STATUS efi_main(EFI_HANDLE imgHandle, EFI_SYSTEM_TABLE *sysTab) {
    InitializeLib(imgHandle, sysTab);

    //initlize variables we will use 
    EFI_STATUS status;

    //gop variables 
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gopinfo;

    //Image Handle
    EFI_LOADED_IMAGE *loaded_img = NULL;
    EFI_GUID loaded_image_proto = LOADED_IMAGE_PROTOCOL;
    EFI_FILE_HANDLE rootfs = NULL;	

    framebfr_t* fb = set_gop_mode(gop, gopinfo);
    Print((CHAR16 *)L"EFI system loaded\r\nVersion: %d.%d.%c\r\n", ver_maj, ver_min, ver_stable);
    rootfs = open_rootfs(imgHandle);
    
    CHAR16* fileName = (CHAR16*) L"kernel.elf";
    EFI_FILE_HANDLE fileHandle;

    //Open the file Kernel.elf
    status = uefi_call_wrapper(rootfs->Open, 5, rootfs, &fileHandle, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if(EFI_ERROR(status)) {
        Print((CHAR16 *)L"Error Opening File %s", fileName);
        while(1);
    }
    UINT64 read_size = get_file_size(fileHandle);
    UINT8 *buffer = AllocatePool(read_size); //allocate a memory pool the size of file 
    
    status = uefi_call_wrapper(fileHandle->Read, 3, fileHandle, &read_size, buffer);
    if(EFI_ERROR(status)) {
        Print((CHAR16*)L"Error Reading from file %s", fileName);
        while(1);
    }
    

    Elf64_Ehdr *hdr = (Elf64_Ehdr*)buffer; //cast buffer of file into 
    
    buffer = buffer + hdr->e_phoff; //set the buffer tot the phdrs location
    Elf64_Phdr *phdrs = (Elf64_Phdr*)buffer; //cast the current buffer pointer into phdrs pointer 
    void *end = buffer + (hdr->e_phnum * hdr->e_phentsize); // set the end point for the for loop 
    buffer = buffer - hdr->e_phoff; //reset buffer postion 
    
    //attempted to simplfy this for loop.
    //seems to still work but may revert if it breaks 
    for (Elf64_Phdr* phdr = phdrs; phdr < (Elf64_Phdr*) end; phdr += hdr->e_phentsize) {
        if(phdr->p_type == PT_LOAD){
            int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;  //caluclate the pages of the kernel
            Elf64_Addr segment = phdr->p_paddr; // kernel segements 
            uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &segment); //allocate the pages for the kernel (Especially important in 64bit where paging is a MUST.)
            uefi_call_wrapper(fileHandle->SetPosition, 2, fileHandle, phdr->p_offset); //Set the kernel file postion offset to the offset of the current phdr 
            UINTN size = phdr->p_filesz; 
            uefi_call_wrapper(fileHandle->Read, 3, fileHandle, &size, (void*)segment);
        }
    }

    

    Print((CHAR16 *)L"0x%016x\r\n", hdr->e_entry);
    
    //Define the kernel start point and make it clear it's a system V ABI not an MS ABI
    int (*kernel_start)(framebfr_t*) = ((__attribute__((sysv_abi)) int (*)()) hdr->e_entry);
    
    //Call the kernels start function 
    kernel_start(fb);
    while(1);
    return EFI_SUCCESS;
}
