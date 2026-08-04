//
// Created by vikto on 11.10.2024.
//

#pragma once

#include <sys/user.h>
#include <sys/mman.h>
#include <stdint.h>
#include <asm-generic/unistd.h>
#include <linux/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

namespace memory {
    bool is_valid_pointer(void *);
    bool protect(uintptr_t, int);

    bool perform_mem_rw(uintptr_t, void*, size_t, bool);

    uintptr_t safe_read(uintptr_t);
    bool safe_write(uintptr_t, uintptr_t);

    int get_protection(uintptr_t address);
}
