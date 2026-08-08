//
// Halalium-style memory: direct LDR/STR in-process (not process_vm_*).
//

#include "memory.hpp"
#include "../halalium_mem.h"

#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE sysconf(_SC_PAGESIZE)

namespace memory {

bool protect(uintptr_t address, int prot)
{
    uintptr_t page_start = (address) & (~(PAGE_SIZE - 1));
    int ret = mprotect((void *)page_start, PAGE_SIZE, prot);
    if (ret != -1)
        hmem::refresh_maps();
    return ret != -1;
}

int get_protection(uintptr_t address)
{
    return hmem::prot_of(address, 1);
}

bool perform_mem_rw(uintptr_t address, void *buffer, size_t size, bool write)
{
    if (!address || !buffer || size == 0)
        return false;

    if (write)
        return hmem::write_bytes(address, buffer, size);

    // Halalium: direct in-process LDR (no hard maps gate)
    std::memcpy(buffer, reinterpret_cast<const void *>(address), size);
    return true;
}

bool is_valid_pointer(void *addr)
{
    if (!addr)
        return false;
    return hmem::readable(reinterpret_cast<uintptr_t>(addr), sizeof(void *));
}

uintptr_t safe_read(uintptr_t address)
{
    return hmem::read_ptr(address);
}

bool safe_write(uintptr_t address, uintptr_t value)
{
    return hmem::write_ptr(address, value);
}

} // namespace memory
