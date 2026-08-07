#pragma once
// Crash-safe in-process memory IO.
// NEVER direct-dereference unknown pointers (that SIGSEGV'd on inject).

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

namespace mem {

inline int self_mem_fd(bool writable) {
    static int rfd = -2;
    static int wfd = -2;
    if (writable) {
        if (wfd == -2) {
            wfd = open("/proc/self/mem", O_RDWR);
            if (wfd < 0) wfd = open("/proc/self/mem", O_WRONLY);
        }
        return wfd;
    }
    if (rfd == -2) rfd = open("/proc/self/mem", O_RDONLY);
    return rfd;
}

inline bool valid_addr(uintptr_t addr, size_t n) {
    if (!addr || n == 0) return false;
    if (addr < 0x10000) return false;
    if (addr > 0x0000FFFFFFFFFFFFULL) return false;
    if (addr + n < addr) return false;
    return true;
}

inline bool read_bytes(uintptr_t addr, void* dst, size_t n) {
    if (!valid_addr(addr, n) || !dst) return false;
    std::memset(dst, 0, n);

    const int fd = self_mem_fd(false);
    if (fd >= 0) {
        const ssize_t r = pread(fd, dst, n, static_cast<off_t>(addr));
        if (r == static_cast<ssize_t>(n)) return true;
    }

    iovec local{dst, n};
    iovec remote{reinterpret_cast<void*>(addr), n};
    const ssize_t r = process_vm_readv(getpid(), &local, 1, &remote, 1, 0);
    if (r == static_cast<ssize_t>(n)) return true;

    // Do NOT memcpy — unmapped addr = instant crash
    return false;
}

inline bool write_bytes(uintptr_t addr, const void* src, size_t n) {
    if (!valid_addr(addr, n) || !src) return false;

    const int fd = self_mem_fd(true);
    if (fd >= 0) {
        const ssize_t w = pwrite(fd, src, n, static_cast<off_t>(addr));
        if (w == static_cast<ssize_t>(n)) return true;
    }

    iovec local{const_cast<void*>(src), n};
    iovec remote{reinterpret_cast<void*>(addr), n};
    const ssize_t w = process_vm_writev(getpid(), &local, 1, &remote, 1, 0);
    return w == static_cast<ssize_t>(n);
    // Do NOT direct-write fallback
}

template <typename T>
inline bool read_into(uintptr_t addr, T& out) {
    return read_bytes(addr, &out, sizeof(T));
}

template <typename T>
inline T read(uintptr_t addr, T fallback = {}) {
    T v{};
    if (!read_into(addr, v)) return fallback;
    return v;
}

template <typename T>
inline bool write(uintptr_t addr, const T& v) {
    return write_bytes(addr, &v, sizeof(T));
}

inline uintptr_t read_ptr(uintptr_t addr) {
    return read<uintptr_t>(addr, 0);
}

inline bool write_ptr(uintptr_t addr, uintptr_t v) {
    return write<uintptr_t>(addr, v);
}

} // namespace mem
