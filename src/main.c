#include <efi.h>
#include <efilib.h>
#include <elf.h> 
#include <utils.h> 
#include <efi-ver.h>

//Remember uefi call wrapper layout
//uefi_call_wrapper(function, argc, args)
//Casting to (CHAR16 *) isn't really needed but it fixes issues of warning showing up in my text editor.
EFI_FILE_HANDLE open_rootfs(EFI_HANDLE img) {
    //Initialize all the variables we will be using and null them
    EFI_LOADED_IMAGE  *loaded_image = NULL;
    EFI_GUID image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_HANDLE loaded_root = NULL;
    EFI_STATUS status = 0;
    
    status = uefi_call_wrapper(BS->HandleProtocol, 3, img, &image_guid, (void **)&loaded_image);
    if(EFI_ERROR(status)) {
	Print((CHAR16*)L"Error BS->HandleProtocol call, image_guid");
	return NULL;
    } else {
	Print((CHAR16*)L"Opened rootfs");
	loaded_root = LibOpenRoot(&loaded_image->DeviceHandle);
	return loaded_root; 
    }
}

EFI_STATUS efi_main(EFI_HANDLE imgHandle, EFI_SYSTEM_TABLE *sysTab) {
    InitializeLib(imgHandle, sysTab);

    //initlize variables we will use 
    EFI_STATUS status;
     
    //gop variables 
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN infosize, num_modes, native_mode;

    //Image Handle
    EFI_LOADED_IMAGE *loaded_img = NULL;
    EFI_GUID loaded_image_proto = LOADED_IMAGE_PROTOCOL;
    EFI_FILE_HANDLE rootfs = NULL;	

    clr_scr();
    Print((CHAR16 *)L"EFI system loaded\r\nVersion: %d.%d.%c\r\n", ver_maj, ver_min, ver_stable);
    
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop);
 
    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &infosize, &info);
    if(status == EFI_NOT_STARTED) {
	status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    } else if(EFI_ERROR(status)) {
	Print((CHAR16*)L"Unable to get native mode");
    } else {
	native_mode = gop->Mode->Mode;
	num_modes = gop->Mode->MaxMode;
    }

    clr_scr(); 

    rootfs = open_rootfs(imgHandle); 
    
    

    while(1) {

    }
    return EFI_SUCCESS;

}
