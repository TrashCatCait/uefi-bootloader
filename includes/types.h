#ifndef CAT_UEFI_TYPES_H
#define CAT_UEFI_TYPES_H

typedef unsigned char uint8_t;

typedef unsigned int uint32_t;

typedef unsigned long uint64_t;

typedef unsigned long long size_t;

typedef struct framebfr {
    void *buffer_base;
    size_t size;
    uint32_t width;
    uint32_t height;
    uint32_t ppsl;
}__attribute__((packed)) framebfr_t;

typedef struct bootinfo {
    char signature[4];
    uint8_t *memmap_loc;
    uint8_t *fontmap;
}__attribute__((packed)) bootinfo_t;

typedef struct efi_memory {
    uint32_t type;
    uint32_t pad;
    uint64_t paddr;
    uint64_t vaddr;
    uint64_t numofPages;
    uint64_t attr;
} efi_memory_t;


#endif 
