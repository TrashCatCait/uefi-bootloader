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

//Sets the screen to the largest resolution GOP mode found
EFI_STATUS set_gop_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info) {
    EFI_STATUS status;
    UINTN infosize, num_modes, native_mode, largest_mode = 0;
    UINT32 largest_h = 0, largest_v = 0;

    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop); //;locate the GOP protocol and file the gop variable with it 
    if(EFI_ERROR(status)) {
        Print((CHAR16*)L"EFI Unable to find gop");
        while(1);
    }
    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &infosize, &info); //query the GOPs current mode 
    if(status == EFI_NOT_STARTED) { //if it's not started  
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0); // start EFI GOP if it s not started 
    } 
    if(EFI_ERROR(status)) {
        Print((CHAR16*)L"Unable to get native mode"); //If an error occurs print erro message
        while(1);
    } else {
        native_mode = gop->Mode->Mode;
        num_modes = gop->Mode->MaxMode;
    }
    clr_scr(); 
    for(UINT64 i = 0; i < num_modes; i++) {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &infosize, &info);
        if(EFI_ERROR(status)) {
            Print((CHAR16 *)L"Unable to Query mode %d", i);
        }
        if(largest_h <= info->HorizontalResolution) { 
            largest_h = info->HorizontalResolution;
            largest_v = info->VerticalResolution;
            largest_mode = i; //save largest screen resolution for use later
        }
    }  
    status = uefi_call_wrapper(gop->SetMode, 2, gop, largest_mode);
    if(EFI_ERROR(status)) {
        Print((CHAR16 *)L"Error Setting mode %03d, %r", largest_mode, status);
        while(1);
    }

    clr_scr();
    Print((CHAR16 *)L"EFI Initialized screen with %dx%d resolution with mode: %d\r\n", gop->Mode->Info->HorizontalResolution, gop->Mode->Info->VerticalResolution, gop->Mode->Mode); 
    return EFI_SUCCESS;
}

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
