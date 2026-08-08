#define	DT_INIT_ARRAY	25
#define	DT_INIT_ARRAYSZ	27

#define	R_AARCH64_ABS64		257	/* Absolute offset */
#define	R_AARCH64_JUMP_SLOT	1026	/* Set GOT entry to code address */
#define	R_AARCH64_RELATIVE 	1027	/* Add load address of shared object */
#define	R_AARCH64_GLOB_DAT 1025

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)

// #include "zygisk/utils.h"


int get_memory_permission(void *address)
{
    //это с гитхаба спастил не помню откуда именно ну короче кал

    unsigned long addr = (unsigned long)address;
    FILE *fp;
    char buf[PATH_MAX];
    char perms[5];
    int bol = 1;

    fp = fopen(("/proc/self/maps"), ("r"));
    if (fp == NULL) {
        return 0;
    }
    while (fgets(buf, PATH_MAX, fp) != NULL) {
        unsigned long start, end;
        int eol = (strchr(buf, '\n') != NULL);
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

        if (sscanf(buf, ("%lx-%lx %4s"), &start, &end, perms) != 3) {
            continue;
        }
        if (start <= addr && addr < end) {
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
    fclose(fp);
    return 0;
    unknown_perms:
    fclose(fp);
    return 0;
}

#define ALLIGN(addr) ((void*)((size_t)(addr) & ~(sysconf(_SC_PAGESIZE) - 1)))

void hook(void *address, void *replace, void **orig)
{
    int prot = get_memory_permission(ALLIGN((void *) address));
    mprotect(ALLIGN(address), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
    if (orig)
    {
        *orig = * (void **) address;
    }
    * (void **) address =  replace;

    mprotect((void *) ALLIGN(address), sysconf(_SC_PAGESIZE), prot);
    return;
}

void plthook(void *address, void *replace)
{
    hook(address, replace, nullptr);
    return;
}

void DelegateHook(void *address, void *hook, void **orig)
{
    void *&addr = * (void **) address;

    while (!addr)
    {
        sleep(1);
    }

    if (orig) { *orig = addr; }

    addr = hook;

    return;
}

#include <fcntl.h>

struct hook_info
{
    bool is_trampoline = false;

    union
    {
        const char *lib_path;
        uintptr_t so_addr;
    };

    union
    {
        const char *s_name;
        void *hook_address;
    };

    union
    {
        void *orig_data;
        void *rel_address;
    };
};

std::vector<hook_info> hooks;

pid_t __target_pid__ = -1;
 
#include <inttypes.h>

namespace utils {
     struct module_info {
        void* start_address = nullptr;
        void* end_address  = nullptr;
        size_t size = 0;
        std::string name;

        int perms;
    };
    bool read_address(void *address, void *out_buffer, size_t length) {
        unsigned long page_size = sysconf(_SC_PAGE_SIZE);
        unsigned long size = page_size * sizeof(uintptr_t);
        return mprotect(
               (void *)((uintptr_t)address - ((uintptr_t)address % page_size) - page_size),
               (size_t)size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && 
               memcpy(out_buffer, address, length) != 0;
    }

    bool write_address(void *address, void *in_buffer, size_t length) {
        unsigned long page_size = sysconf(_SC_PAGE_SIZE);
        unsigned long size = page_size * sizeof(uintptr_t);
        return mprotect(
               (void *)((uintptr_t)address - ((uintptr_t)address % page_size) - page_size),
               (size_t)size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 &&
               memcpy(address, in_buffer, length) != 0;
    }

    std::vector<module_info> find_modules(void *address) {
        std::vector<module_info> modules;
        char line[512];

        FILE *f = fopen(oxorany("/proc/self/maps"), oxorany("rt"));

        if (!f) return modules;

        module_info module;

        while (fgets(line, sizeof line, f)) {
            uintptr_t tmpBase;
            uintptr_t tmpEnd;
            char tmpName[256];
            char tmpPerms[4];
            if (sscanf(line, "%" PRIXPTR "-%" PRIXPTR "%s %*s %*s %*s %s", &tmpBase, &tmpEnd, tmpPerms, tmpName) > 0) {
                if (address == reinterpret_cast<void*>(tmpBase) || module.end_address == reinterpret_cast<void*>(tmpBase)) {
                    module.start_address = reinterpret_cast<void*>(tmpBase);
                    module.end_address = reinterpret_cast<void*>(tmpEnd);
                    module.size = tmpEnd - tmpBase;
                    module.name = std::string(tmpName);

                    int perms{};

                    if (tmpPerms[0] == 'r')
                    {
                        perms |= PROT_READ;
                    }

                    if (tmpPerms[1] == 'w')
                    {
                        perms |= PROT_WRITE;
                    }

                    if (tmpPerms[2] == 'x')
                    {
                        perms |= PROT_EXEC;
                    }

                    module.perms = perms;

                    module_info new_module = module;
                    modules.push_back(new_module);

                    if (strstr(tmpName, oxorany(".bss")))
                    {
                        break;
                    }
                }
            }
        }
        fclose(f);
        return modules;
    }

    module_info find_module(const char *name) {
        module_info module{};
        char line[512];

        FILE *f = fopen(oxorany("/proc/self/maps"), oxorany("rt"));

        if (!f) return module;

        while (fgets(line, sizeof line, f)) {
            uintptr_t tmpBase;
            uintptr_t tmpEnd;
            char tmpName[256];
            char tmpPerms[4]; 
            if (sscanf(line, "%" PRIXPTR "-%" PRIXPTR "%s %*s %*s %*s %s", &tmpBase, &tmpEnd, tmpPerms, tmpName) > 0) {
                if (strstr(line, name) && strstr(line, oxorany("-xp"))) {
                    module.start_address = reinterpret_cast<void*>(tmpBase);
                    module.end_address = reinterpret_cast<void*>(tmpEnd);
                    module.size = tmpEnd - tmpBase;
                    module.name = std::string(tmpName);
                    break;
                }
            }
        }
        fclose(f);
        return module;
    }
    
    uintptr_t find_pattern(uint8_t* start, const size_t length, const char* pattern) {
        const char* pat = pattern;
        uint8_t* first_match = 0;
        for (auto current_byte = start; current_byte < (start + length); ++current_byte) {
            if (*pat == '?' || *current_byte == strtoul(pat, NULL, 16)) {
                if (!first_match)
                    first_match = current_byte;
                if (!pat[2])
                    return (uintptr_t)first_match;
                pat += *(uint16_t*)pat == 16191 || *pat != '?' ? 3 : 2;
            }
            else if (first_match) {
                current_byte = first_match;
                pat = pattern;
                first_match = 0;
            }
        } return 0;
    }
    uintptr_t find_pattern_in_module(const char* lib_name, const char* pattern) {
        module_info mod = find_module(lib_name);
        return find_pattern((uint8_t*)mod.start_address, mod.size, pattern);
    }
}

bool plthook(const char *so_name, const char *import_function_name, void *replacement)
{
    uintptr_t so_addr = (uintptr_t) utils::find_module(so_name).start_address;
    if (so_addr)
    {
        Dl_info info;
        if (dladdr((void *) so_addr, &info))
        {
            int fd = open(info.dli_fname, O_RDONLY);
            if (fd != -1)
            {
                int size = lseek(fd, 0, SEEK_END);
                if (size <= 0)
                {
                    // loge("[-] lseek() failed for %s", info.dli_fname);

                    return false;
                }

                auto header = (Elf64_Ehdr *) (mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));

                close(fd);

                auto* shdr = (Elf64_Shdr *)((char *) header + header->e_shoff);
                Elf64_Shdr* dynamic_shdr = nullptr;
                for (int i = 0; i < header->e_shnum; i++)
                {
                    if (shdr[i].sh_type == SHT_DYNAMIC)
                    {
                        dynamic_shdr = &shdr[i];

                        break;
                    }
                }

                if (!dynamic_shdr)
                {
                    // loge("[-] Failed to find DYNAMIC section");

                    return false;
                }

                auto* phdr = (Elf64_Phdr *)((char *) header + header->e_phoff);

                size_t pt_dynamic_offset = 0;
                size_t pt_dynamic_filesz = 0;
                for (int i = 0; i < header->e_phnum; i++)
                {
                    if (phdr[i].p_type == PT_DYNAMIC)
                    {
                        pt_dynamic_offset = phdr[i].p_offset;
                        pt_dynamic_filesz = phdr[i].p_filesz;
                    }
                }

                Elf64_Shdr* strtab_shdr = &shdr[dynamic_shdr->sh_link];

                auto* dynamic = (Elf64_Dyn *) ((char *) header + dynamic_shdr->sh_offset);
                const char* strtab = (const char*) (char *) header + strtab_shdr->sh_offset;
                size_t strtab_size = strtab_shdr->sh_size;

                Elf64_Addr load_bias = so_addr;

                Elf64_Sym* symtab = nullptr;
                Elf64_Addr* init_array = nullptr;
                size_t init_array_count = 0;

                Elf64_Rela* rela = nullptr;
                Elf64_Rela* plt_rela = nullptr;
                size_t rela_count = 0;
                size_t plt_rela_count = 0;

                for (int i = 0; dynamic[i].d_tag != DT_NULL; i++)
                {
                    if (dynamic[i].d_tag == DT_RELA)
                    {
                        rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_RELASZ)
                    {
                        rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_JMPREL)
                    {
                        plt_rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_PLTRELSZ)
                    {
                        plt_rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_SYMTAB)
                    {
                        symtab = (Elf64_Sym *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                }

                for (int k = 0; k < plt_rela_count; k++)
                {
                    uint32_t r_sym = ELF64_R_SYM(plt_rela[k].r_info);
                    uint32_t r_type = ELF64_R_TYPE(plt_rela[k].r_info);
                    if (r_type == R_AARCH64_JUMP_SLOT)
                    {
                        Elf64_Sym* symbol = &symtab[r_sym];
                        const char* symbol_name = strtab + symbol->st_name;

                        if (!strcmp(symbol_name, import_function_name))
                        {
                            void *rel_target = (void*) (load_bias + plt_rela[k].r_offset);
                            void *&rel_addr = *(void**)rel_target;

                            hook_info hk_info = {
                                    .lib_path = info.dli_fname,
                                    .s_name = symbol_name,
                                    .rel_address = rel_addr
                            };

                            hooks.push_back(hk_info);
                            int orig_perms = get_memory_permission(ALLIGN(rel_target));

                            // LOGD("[+] Found %s, orig address %p, perms %d", import_function_name, *(void**)rel_target, orig_perms);

                            mprotect(ALLIGN(rel_target), PAGE_SIZE, 3);
                            rel_addr = replacement;
                            mprotect(ALLIGN(rel_target), PAGE_SIZE, orig_perms);

                            return true;
                        }
                    }
                }
            }
            else
            {
                // loge("[-] Library not opened, fd = -1");
            }
        }
        else
        {
            // loge("[-] Failed to get info of address");
        }
    }
    else
    {
        // loge("[-] Failed to get so address");
    }

    return false;
}

bool print_info(const char *so_name, const char *import_function_name)
{
    uintptr_t so_addr = (uintptr_t) utils::find_module(so_name).start_address;
    if (so_addr)
    {
        Dl_info info;
        if (dladdr((void *) so_addr, &info))
        {
            int fd = open(info.dli_fname, O_RDONLY);
            if (fd != -1)
            {
                int size = lseek(fd, 0, SEEK_END);
                if (size <= 0)
                {
                    // loge("[-] lseek() failed for %s", info.dli_fname);

                    return false;
                }

                auto header = (Elf64_Ehdr *) (mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));

                close(fd);

                auto* shdr = (Elf64_Shdr *)((char *) header + header->e_shoff);
                Elf64_Shdr* dynamic_shdr = nullptr;
                for (int i = 0; i < header->e_shnum; i++)
                {
                    if (shdr[i].sh_type == SHT_DYNAMIC)
                    {
                        dynamic_shdr = &shdr[i];

                        break;
                    }
                }

                if (!dynamic_shdr)
                {
                    // loge("[-] Failed to find DYNAMIC section");

                    return false;
                }

                auto* phdr = (Elf64_Phdr *)((char *) header + header->e_phoff);

                size_t pt_dynamic_offset = 0;
                size_t pt_dynamic_filesz = 0;
                for (int i = 0; i < header->e_phnum; i++)
                {
                    if (phdr[i].p_type == PT_DYNAMIC)
                    {
                        pt_dynamic_offset = phdr[i].p_offset;
                        pt_dynamic_filesz = phdr[i].p_filesz;
                    }
                }

                Elf64_Shdr* strtab_shdr = &shdr[dynamic_shdr->sh_link];

                auto* dynamic = (Elf64_Dyn *) ((char *) header + dynamic_shdr->sh_offset);
                const char* strtab = (const char*) (char *) header + strtab_shdr->sh_offset;
                size_t strtab_size = strtab_shdr->sh_size;

                Elf64_Addr load_bias = so_addr;

                Elf64_Sym* symtab = nullptr;
                Elf64_Addr* init_array = nullptr;
                size_t init_array_count = 0;

                Elf64_Rela* rela = nullptr;
                Elf64_Rela* plt_rela = nullptr;
                size_t rela_count = 0;
                size_t plt_rela_count = 0;

                for (int i = 0; dynamic[i].d_tag != DT_NULL; i++)
                {
                    if (dynamic[i].d_tag == DT_RELA)
                    {
                        rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_RELASZ)
                    {
                        rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_JMPREL)
                    {
                        plt_rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_PLTRELSZ)
                    {
                        plt_rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_SYMTAB)
                    {
                        symtab = (Elf64_Sym *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                }

                for (int k = 0; k < plt_rela_count; k++)
                {
                    uint32_t r_sym = ELF64_R_SYM(plt_rela[k].r_info);
                    uint32_t r_type = ELF64_R_TYPE(plt_rela[k].r_info);
                    if (r_type == R_AARCH64_JUMP_SLOT)
                    {
                        Elf64_Sym* symbol = &symtab[r_sym];
                        const char* symbol_name = strtab + symbol->st_name;

                        if (true)
                        {
                            void *rel_target = (void*) (load_bias + plt_rela[k].r_offset);
                            void *&rel_addr = *(void**)rel_target;
                            Dl_info info;
                            dladdr(rel_addr, &info);
                            // LOGD("base name: %s\nbase addr: %p\nsym name: %s", info.dli_fname, info.dli_fbase, info.dli_sname);

                            return true;
                        }
                    }
                }
            }
            else
            {
                // loge("[-] Library not opened, fd = -1");
            }
        }
        else
        {
            // loge("[-] Failed to get info of address");
        }
    }
    else
    {
        // loge("[-] Failed to get so address");
    }

    return false;
}

void* get_plt_rel_addr(const char *so_name, const char *import_function_name)
{
    uintptr_t so_addr = (uintptr_t) utils::find_module(so_name).start_address;
    if (so_addr)
    {
        Dl_info info;
        if (dladdr((void *) so_addr, &info))
        {
            int fd = open(info.dli_fname, O_RDONLY);
            if (fd != -1)
            {
                int size = lseek(fd, 0, SEEK_END);
                if (size <= 0)
                {
                    // loge("[-] lseek() failed for %s", info.dli_fname);

                    return nullptr;
                }

                auto header = (Elf64_Ehdr *) (mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));

                close(fd);

                auto* shdr = (Elf64_Shdr *)((char *) header + header->e_shoff);
                Elf64_Shdr* dynamic_shdr = nullptr;
                for (int i = 0; i < header->e_shnum; i++)
                {
                    if (shdr[i].sh_type == SHT_DYNAMIC)
                    {
                        dynamic_shdr = &shdr[i];
                        break;
                    }
                }

                if (!dynamic_shdr)
                {
                    // loge("[-] Failed to find DYNAMIC section");

                    return nullptr;
                }

                auto* phdr = (Elf64_Phdr *)((char *) header + header->e_phoff);

                size_t pt_dynamic_offset = 0;
                size_t pt_dynamic_filesz = 0;
                for (int i = 0; i < header->e_phnum; i++)
                {
                    if (phdr[i].p_type == PT_DYNAMIC)
                    {
                        pt_dynamic_offset = phdr[i].p_offset;
                        pt_dynamic_filesz = phdr[i].p_filesz;
                    }
                }

                Elf64_Shdr* strtab_shdr = &shdr[dynamic_shdr->sh_link];

                auto* dynamic = (Elf64_Dyn *) ((char *) header + dynamic_shdr->sh_offset);
                const char* strtab = (const char*) (char *) header + strtab_shdr->sh_offset;
                size_t strtab_size = strtab_shdr->sh_size;

                Elf64_Addr load_bias = so_addr;

                Elf64_Sym* symtab = nullptr;
                Elf64_Addr* init_array = nullptr;
                size_t init_array_count = 0;

                Elf64_Rela* rela = nullptr;
                Elf64_Rela* plt_rela = nullptr;
                size_t rela_count = 0;
                size_t plt_rela_count = 0;

                for (int i = 0; dynamic[i].d_tag != DT_NULL; i++)
                {
                    if (dynamic[i].d_tag == DT_RELA)
                    {
                        rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);

                    }
                    if (dynamic[i].d_tag == DT_RELASZ)
                    {
                        rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_JMPREL)
                    {
                        plt_rela = (Elf64_Rela *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_PLTRELSZ)
                    {
                        plt_rela_count = dynamic[i].d_un.d_val / sizeof(Elf64_Rela);
                    }
                    if (dynamic[i].d_tag == DT_SYMTAB)
                    {
                        symtab = (Elf64_Sym *)((char *) header + dynamic[i].d_un.d_ptr);
                    }
                    if (dynamic[i].d_tag == DT_INIT_ARRAY)
                    {
                        init_array = (Elf64_Addr *)(load_bias + dynamic[i].d_un.d_ptr); // DT_INIT_ARRAY will be relocated!!! So we will use it from address space of our mapped lib not ours where we mapped file into our memory
                    }
                    if (dynamic[i].d_tag == DT_INIT_ARRAYSZ)
                    {
                        init_array_count = dynamic[i].d_un.d_val / sizeof(Elf64_Addr);
                    }
                }

                // LOGD("[+] Processing hooking (DT_JMPREL)...");

                for (int k = 0; k < plt_rela_count; k++)
                {
                    uint32_t r_sym = ELF64_R_SYM(plt_rela[k].r_info);
                    uint32_t r_type = ELF64_R_TYPE(plt_rela[k].r_info);
                    if (r_type == R_AARCH64_JUMP_SLOT)
                    {
                        Elf64_Sym* symbol = &symtab[r_sym];
                        const char* symbol_name = strtab + symbol->st_name;

                        if (!strcmp(symbol_name, import_function_name))
                        {
                            void *rel_target = (void*) (load_bias + plt_rela[k].r_offset);
                            void *rel_addr = *(void**)rel_target;

                            return rel_addr;
                        }
                    }
                }
            }
            else
            {
                // loge("[-] Library not opened, fd = -1");
            }
        }
        else
        {
            // loge("[-] Failed to get info of address");
        }
    }
    else
    {
        // loge("[-] Failed to get so address");
    }

    return nullptr;
}