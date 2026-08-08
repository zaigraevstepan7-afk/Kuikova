//
// Created by vikto on 11.10.2024.
//

#include "memory.hpp"
#include <bits/page_size.h>
#include <unistd.h>
#include <sys/mman.h>
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

//extern "C" long raw_syscall(long __number, ...);

namespace memory {
    bool protect(uintptr_t address, int prot) {
        uintptr_t page_start = (address) & (~(PAGE_SIZE - 1));
        int ret = mprotect((void *) page_start, PAGE_SIZE, prot);

        return ret != -1;
    }

    int get_protection(uintptr_t address) {
        char line[PATH_MAX] = {0};
        char perms[5];
        int bol = 1;

        auto fp = fopen("/proc/self/maps", "r");
        if (fp == nullptr) {
            return 0;
        }

        while (fgets(line, PATH_MAX, fp) != nullptr) {
            uint64_t start, end;
            int eol = (strchr(line, '\n') != nullptr);
            if (bol) {
                if (!eol) {
                    bol = 0;
                }
            } else {
                if (eol) {
                    bol = 1;
                }
                continue;
            }
            if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) != 3) {
                continue;
            }
            if (start <= address && address < end) {
                int prot = 0;
                if (perms[0] == 'r') {
                    prot |= PROT_READ;
                } else if (perms[0] != '-') {
                    goto unknown_perms;
                }
                if (perms[1] == 'w') {
                    prot |= PROT_WRITE;
                } else if (perms[1] != '-') {
                    goto unknown_perms;
                }
                if (perms[2] == 'x') {
                    prot |= PROT_EXEC;
                } else if (perms[2] != '-') {
                    goto unknown_perms;
                }
                if (perms[3] != 'p') {
                    goto unknown_perms;
                }
                if (perms[4] != '\0') {
                    perms[4] = '\0';
                    goto unknown_perms;
                }
                fclose(fp);
                return prot;
            }
        }
        unknown_perms:
        fclose(fp);
        return 0;
    }

    bool perform_mem_rw(uintptr_t address, void* buffer, size_t size, bool write) {
        struct iovec local{}, remote{};
        local.iov_base = buffer;
        local.iov_len = size;
        remote.iov_base = reinterpret_cast<void *>(address);
        remote.iov_len = size;
        //return raw_syscall(write ? __NR_process_vm_writev : __NR_process_vm_readv, getpid(), &local, 1, &remote, 1, 0) == size;
        return syscall(write ? __NR_process_vm_writev : __NR_process_vm_readv, getpid(), &local, 1, &remote, 1, 0) == size;
    }

    bool is_valid_pointer(void * addr) {
        static int random_fd = 0;

        if (!addr) return false;
        /*if (random_fd <= 0)
            random_fd = raw_syscall(__NR_memfd_create, "jit-cache",MFD_CLOEXEC | MFD_ALLOW_SEALING);
        return raw_syscall(__NR_write, random_fd, addr, 4) >= 0;*/
        if (random_fd <= 0)
            random_fd = syscall(__NR_memfd_create, "jit-cache", MFD_CLOEXEC | MFD_ALLOW_SEALING);
        return syscall(__NR_write, random_fd, addr, 4) >= 0;
    }

    uintptr_t safe_read(uintptr_t address) {
        uintptr_t value = 0;
        if (!perform_mem_rw(address, &value, sizeof(uintptr_t),false)) {
            return false;
        }
        return value;
    }

    bool safe_write(uintptr_t address, uintptr_t value) {
        return perform_mem_rw(address, &value, sizeof(value),true);
    }
}