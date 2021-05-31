#include <efi/efi.h>
#include <efi/efilib.h>
#include <elf.h> 
#include "../includes/efi-loader.h"

//Remember uefi call wrapper layout
//uefi_call_wrapper(function, argc, args)
//Casting to (CHAR16 *) isn't really needed but it fixes issues of warning showing up in my text editor.

typedef unsigned long long size_t; // 64 bit unsigned number.
const static CHAR16* welcomeMsg = (CHAR16 *)L"Cait's EFI system loader\r\nVersion: %d.%d.%c\r\n";

//Get volume to load files from. We use the imagehandle from efi main to get the ESP partition.
//This application was loaded from. Then we use libOpenRoot to open the root volume in a file handle
EFI_FILE_HANDLE getVolume(EFI_HANDLE image)
{
    EFI_LOADED_IMAGE *loadedImage = NULL; // Loaded Image interface used for opening device
    EFI_GUID imageGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID; // Image GUID interface  
    
    //populate our loaded image
    uefi_call_wrapper(BS->HandleProtocol, 3, image, &imageGUID, (void **) &loadedImage);

    EFI_FILE_HANDLE loadedRoot; //loaded root partition volume 
    
    //Use this instead of IOVOLUME
    loadedRoot = LibOpenRoot(loadedImage->DeviceHandle);

    //Return our loaded root file
    return loadedRoot;
}

UINT8 *load_kernel(EFI_FILE_HANDLE root, CHAR16* fileName){
    //HipptyHoopity Todo:
    EFI_FILE* loadedKernel = NULL;
    UINT64 fileSize;
    EFI_FILE_HANDLE kernelFileHandle;
    
    //Open our file
    uefi_call_wrapper(root->Open, 5, root, &kernelFileHandle, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    
    //Get the files actual size
    EFI_FILE_INFO* fileInfo = LibFileInfo(kernelFileHandle); //File information sturcuture. 
    fileSize = fileInfo->FileSize; //48644864
    FreePool(fileInfo); //Free it as we are done with it

    UINT8 *buffer = AllocatePool(fileSize); //AllocatePool for reading our file into 
    //read the file
    uefi_call_wrapper(kernelFileHandle->Read, 3, kernelFileHandle, &fileSize, buffer);
    Print((CHAR16*) L"0x%x\r\n", buffer[73]); 
    

    return buffer;
}

//This code is not effcient at all but is easy to test if it works with. 
//NOTE TO SELF REPLACE THIS FUNCTION 

//This will be moved into it's own file. Basically the idea is to produce cleaner looking code.
void clr_scr()
{
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
}

typedef struct {
    UINTN memMapSize;
    UINTN descriptSize;
    UINTN mapKey;
    EFI_MEMORY_DESCRIPTOR *memMap;
    UINT32 descriptVer;
} memoryMap;

EFI_STATUS efi_main (EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) 
{
    //InitializeLib efi libary system table and image handle 
    InitializeLib(imageHandle, systemTable);
    //Clear the screen of current contents. As it could still have left overs from the firmware manaufacter such as the motherboard logo.
    
    //Testin something
    EFI_STATUS status; 
    
    clr_scr(); 
    Print(welcomeMsg, EFILOADER_VER_MAJ, EFILOADER_VER_MIN, EFILOADER_STABLE);
    //Open our ESP partions volume. 
    EFI_FILE_HANDLE rootPart = getVolume(imageHandle);
    //Load Kernel File into memory from system ESP partion
    UINT8* kerImg = load_kernel(rootPart, (CHAR16 *)L"kernel.elf");
    
    if(kerImg == NULL) Print((CHAR16 *)L"Unable to load kernel image\r\n");
    else Print((CHAR16 *)L"Kernel successfully loaded\r\n");
    
    //Currently working directly off the kernels offset future plan is to have this changed to work with the kernels elf header to tell what this should be. But having it as a hard coded var makes testing a little easier
    kerImg += 4096;
    
    //Define the kernel Start
    int(*KernelStart)(memoryMap*) = ((__attribute__((sysv_abi)) int(*)() ) kerImg); 

    //InitializeLib some vars we will use to get memory map
    memoryMap memory;
    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5 memory.memMapSize, memory.memMap, NULL, memory.descriptSize, NULL);
    
    //Fill in the memory map size;
    memory.memMapSize += 2 * memory.descriptSize;

    //AllocatePool for our memory map using our defined size from before.
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, memory.memMapSize, (void**)memory.memMap);
    

    //Get the actual memory map
    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, memory.memMapSize, memory.memMap, memory.mapKey, memory.descriptSize, memory.descriptVer);

    Print((CHAR16 *)L"Loaded Kernel and obtained memory map continue boot process?\r\nPress any key to continue...\r\n");
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);

    uefi_call_wrapper(ST->BootServices->ExitBootServices, 2, imageHandle, memory.mapKey);    

    //Call our kernel 
    Print((CHAR16 *)L"%d\r\n", KernelStart(&memory));
    
    uefi_call_wrapper(RT->ResetSystem, 1, EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    
    return EFI_SUCCESS;
}
