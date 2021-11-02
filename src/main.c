#include <efi.h>
#include <efilib.h>
#include <elf.h> 
#include <utils.h> 
#include <efi-ver.h>
#include <gop.h>
#include <rootfs.h>

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

    set_gop_mode(gop, gopinfo);
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
    

    Elf64_Ehdr *header = (Elf64_Ehdr*)buffer;
    
    Elf64_Phdr* phdrs;
    {
        uefi_call_wrapper(fileHandle->SetPosition, 2, fileHandle, header->e_phoff);
        UINTN size = header->e_phnum * header->e_phentsize;
        uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, size, (void**)&phdrs);
        uefi_call_wrapper(fileHandle->Read, 3, fileHandle, &size, phdrs);
    }

    for (
        Elf64_Phdr* phdr = phdrs;
        (char*)phdr < (char*)phdrs + header->e_phnum * header->e_phentsize;
        phdr = (Elf64_Phdr*)((char*)phdr + header->e_phentsize)
    ) {
        switch (phdr->p_type){
            case PT_LOAD:
            {
                int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
                Elf64_Addr segment = phdr->p_paddr;
                uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &segment);

                uefi_call_wrapper(fileHandle->SetPosition, 2, fileHandle, phdr->p_offset);
                UINTN size = phdr->p_filesz;
                uefi_call_wrapper(fileHandle->Read, 3, fileHandle, &size, (void*)segment);
                break;
            }
        }
    }

    

    Print((CHAR16 *)L"0x%x\r\n", header->e_entry);
     
    int (*kernel_start)() = ((__attribute__((sysv_abi)) int (*)()) header->e_entry);
    
    Print((CHAR16*)L"%d",kernel_start());
    while(1);
    return EFI_SUCCESS;
}
