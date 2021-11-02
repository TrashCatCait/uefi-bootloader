#include <rootfs.h>
#include <efilib.h>
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

