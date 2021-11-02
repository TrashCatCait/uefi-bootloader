#include <efi.h>
#include <efilib.h>
#include <elf.h> 
#include <utils.h> 
#include <efi-ver.h>
#include <gop.h>
#include <rootfs.h>

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

    while(1) {

    }
return EFI_SUCCESS;
}
