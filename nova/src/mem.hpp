#pragma once
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/uio.h>

namespace mem {

template <typename T>
inline bool read_into(uintptr_t addr, T& out) {
    if (!addr) return false;
    iovec local{&out, sizeof(T)};
    iovec remote{reinterpret_cast<void*>(addr), sizeof(T)};
    const ssize_t r = process_vm_readv(getpid(), &local, 1, &remote, 1, 0);
    return r == static_cast<ssize_t>(sizeof(T));
}

template <typename T>
inline T read(uintptr_t addr, T fallback = {}) {
    T v{};
    if (!read_into(addr, v)) return fallback;
    return v;
}

inline uintptr_t read_ptr(uintptr_t addr) {
    return read<uintptr_t>(addr, 0);
}

inline bool write_bytes(uintptr_t addr, const void* src, size_t n) {
    if (!addr || !src || !n) return false;
    iovec local{const_cast<void*>(src), n};
    iovec remote{reinterpret_cast<void*>(addr), n};
    return process_vm_writev(getpid(), &local, 1, &remote, 1, 0) == static_cast<ssize_t>(n);
}

} // namespace mem
