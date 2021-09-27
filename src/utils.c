#include <utils.h>


EFI_STATUS check_error(EFI_STATUS status, CHAR16 *action)
{
    if(status != EFI_SUCCESS) {
        Print((CHAR16 *)L"EFI call: %s failed.\r\nWith error: %r", action, status);
        while(1){}
    }
    return EFI_SUCCESS;
}


void clr_scr()
{
    EFI_STATUS status;
    
    status = uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    if(EFI_ERROR(status)) {
	Print((CHAR16 *)L"Error clearing screen: %r", status);
    }
}
