#include <efi.h>
#include <efilib.h>
#include <utils.h>
#include <types.h>

//Sets the screen to the largest resolution GOP mode found
framebfr_t* set_gop_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info) {
    //EFI status return to check for errros 
    EFI_STATUS status;

    //Initialize all vars we will need
    UINTN infosize, num_modes, native_mode, largest_mode = 0;
    UINT32 largest_h = 0, largest_v = 0;
    framebfr_t *ret;
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
    
    ret->buffer_base = (void *)gop->Mode->FrameBufferBase;
    ret->size = gop->Mode->FrameBufferSize;
    ret->height = gop->Mode->Info->VerticalResolution;
    ret->width = gop->Mode->Info->HorizontalResolution;
    ret->ppsl = gop->Mode->Info->PixelsPerScanLine;
    
    return ret;
}
