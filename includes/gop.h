#ifndef INCLUDES_GOP_H
#define INCLUDES_GOP_H

#include <efi.h>
#include <types.h>


framebfr_t* set_gop_mode(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info); 

#endif
