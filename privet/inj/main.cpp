#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <cerrno>

#define PGSZ ((uint64_t)sysconf(_SC_PAGESIZE))
static inline uint64_t alu(uint64_t x){ return (x+PGSZ-1)&~(PGSZ-1); }
static inline uint64_t ald(uint64_t x){ return x&~(PGSZ-1); }

static int mfd=-1;

static void logf(const char* f,...){
    va_list ap; va_start(ap,f);
    vfprintf(stdout,f,ap); va_end(ap);
    fflush(stdout);
}

// aarch64 user regs
struct pr64{ uint64_t regs[31]; uint64_t sp; uint64_t pc; uint64_t pstate; };

static int getregs(int pid,pr64* r){
    iovec iov; iov.iov_base=r; iov.iov_len=sizeof(*r);
    errno=0;
    return ptrace(PTRACE_GETREGSET,pid,(void*)NT_PRSTATUS,&iov)==0?0:-1;
}
static int setregs(int pid,pr64* r){
    iovec iov; iov.iov_base=r; iov.iov_len=sizeof(*r);
    errno=0;
    return ptrace(PTRACE_SETREGSET,pid,(void*)NT_PRSTATUS,&iov)==0?0:-1;
}

static uintptr_t rcall(int pid,uintptr_t fn,int n,uintptr_t a0=0,uintptr_t a1=0,uintptr_t a2=0,uintptr_t a3=0,uintptr_t a4=0,uintptr_t a5=0){
    if(ptrace(PTRACE_ATTACH,pid,0,0)==-1){ logf("[-] attach %s\n",strerror(errno)); return (uintptr_t)-1; }
    int st=0;
    for(int k=0;k<200;k++){ waitpid(pid,&st,__WALL); if((st&0xff)==0x7f) break; }
    pr64 r_,o;
    if(getregs(pid,&r_)==-1){ ptrace(PTRACE_DETACH,pid,0,0); return (uintptr_t)-1; }
    o=r_;
    uintptr_t a[6]={a0,a1,a2,a3,a4,a5};
    for(int i=0;i<n&&i<6;i++) r_.regs[i]=a[i];
    r_.pc=fn;
    r_.regs[30]=0;
    if(setregs(pid,&r_)==-1){ ptrace(PTRACE_DETACH,pid,0,0); return (uintptr_t)-1; }
    if(ptrace(PTRACE_CONT,pid,0,0)==-1){ ptrace(PTRACE_DETACH,pid,0,0); return (uintptr_t)-1; }
    for(int k=0;k<400;k++){
        waitpid(pid,&st,__WALL);
        if((st&0xff)==0x7f) break;
        if(ptrace(PTRACE_CONT,pid,0,0)==-1) break;
    }
    pr64 res;
    if(getregs(pid,&res)==-1){ ptrace(PTRACE_DETACH,pid,0,0); return (uintptr_t)-1; }
    uintptr_t ret=res.regs[0];
    setregs(pid,&o);
    ptrace(PTRACE_DETACH,pid,0,0);
    return ret;
}

static ssize_t rread(uint64_t a,void* b,size_t n){ return pread(mfd,b,n,(off_t)a); }
static ssize_t rwrite(uint64_t a,const void* b,size_t n){ return pwrite(mfd,b,n,(off_t)a); }

struct seg{ uint64_t va,fsz,msz,off,al; uint32_t fl; };
static uint64_t v2o(const std::vector<seg>& S,uint64_t va){
    for(size_t i=0;i<S.size();i++) if(va>=S[i].va&&va<S[i].va+S[i].msz) return S[i].off+(va-S[i].va);
    return 0;
}

// local copy of a remote lib (same file on device), resolves symbols to remote addresses
struct resolver{
    uint64_t base=0;
    std::vector<uint8_t> d;
    std::vector<seg> S;
    uint64_t symtab=0,strtab=0,strsz=0,nsym=0;
    bool ok=false;
    bool load(const std::string& path,uint64_t base_){
        base=base_; ok=false;
        int fd=open(path.c_str(),O_RDONLY);
        if(fd<0) return false;
        off_t sz=lseek(fd,0,SEEK_END); lseek(fd,0,SEEK_SET);
        if(sz<=0){ close(fd); return false; }
        d.resize((size_t)sz);
        if(read(fd,d.data(),(size_t)sz)!=(ssize_t)sz){ close(fd); return false; }
        close(fd);
        if(d.size()<sizeof(Elf64_Ehdr)) return false;
        Elf64_Ehdr* eh=(Elf64_Ehdr*)d.data();
        if(memcmp(eh->e_ident,"\x7f""ELF",4)||eh->e_ident[EI_CLASS]!=ELFCLASS64) return false;
        S.clear();
        Elf64_Phdr* ph=(Elf64_Phdr*)(d.data()+eh->e_phoff);
        for(int i=0;i<eh->e_phnum;i++) if(ph[i].p_type==PT_LOAD) S.push_back({ph[i].p_vaddr,ph[i].p_filesz,ph[i].p_memsz,ph[i].p_offset,ph[i].p_align,ph[i].p_flags});
        uint64_t dyn=0,dynsz=0;
        for(int i=0;i<eh->e_phnum;i++) if(ph[i].p_type==PT_DYNAMIC){ dyn=ph[i].p_vaddr; dynsz=ph[i].p_memsz; break; }
        if(!dyn||!dynsz) return false;
        Elf64_Dyn* D=(Elf64_Dyn*)(d.data()+v2o(S,dyn));
        size_t dn=dynsz/16;
        for(size_t i=0;i<dn;i++){
            switch(D[i].d_tag){
                case DT_SYMTAB: symtab=D[i].d_un.d_ptr; break;
                case DT_STRTAB: strtab=D[i].d_un.d_ptr; break;
                case DT_STRSZ: strsz=D[i].d_un.d_val; break;
                case DT_HASH:{ uint32_t* h=(uint32_t*)(d.data()+v2o(S,D[i].d_un.d_ptr)); nsym=h[1]; break; }
                case DT_GNU_HASH:{
                    uint32_t* g=(uint32_t*)(d.data()+v2o(S,D[i].d_un.d_ptr));
                    uint32_t nb=g[0],so=g[1],bs=g[2];
                    uint32_t* buckets=(uint32_t*)((uint64_t)g+16+bs*8);
                    uint32_t* chain=buckets+nb;
                    uint32_t mx=0;
                    for(uint32_t b=0;b<nb;b++){
                        uint32_t ix=buckets[b];
                        if(!ix) continue;
                        while(1){ if(ix>mx) mx=ix; uint32_t ch=chain[ix-so]; ix++; if(ch&1) break; }
                    }
                    nsym=mx+1;
                    break;
                }
            }
        }
        ok=(symtab&&strtab&&strsz);
        return ok;
    }
    uint64_t find(const char* nm) const{
        if(!ok) return 0;
        size_t lim=nsym?nsym:(strsz);
        uint64_t st_off=v2o(S,symtab);
        uint64_t st_off2=v2o(S,strtab);
        if(!st_off||!st_off2) return 0;
        for(size_t i=0;i<lim;i++){
            uint64_t eo=st_off+i*24;
            if(eo+24>d.size()) break;
            Elf64_Sym* s=(Elf64_Sym*)(d.data()+eo);
            if(!s->st_name||!s->st_shndx) continue;
            if(s->st_name>=strsz) continue;
            const char* n=(const char*)(d.data()+st_off2+s->st_name);
            if(!strcmp(n,nm)) return base+s->st_value;
        }
        return 0;
    }
};

static std::string base_of(const std::string& p){
    size_t k=p.find_last_of('/');
    return k==std::string::npos?p:p.substr(k+1);
}

static void collect_libs(int pid,std::map<std::string,std::pair<uint64_t,std::string>>& out){
    char mp[64]; snprintf(mp,64,"/proc/%d/maps",pid);
    FILE* f=fopen(mp,"r");
    if(!f) return;
    char line[1024];
    while(fgets(line,sizeof(line),f)){
        uint64_t st=0,en=0,off=0;
        char perms[8]={0},path[512]={0},dev[16]={0};
        int ino=0;
        if(sscanf(line,"%lx-%lx %7s %lx %15s %d %511s",&st,&en,perms,&off,dev,&ino,path)<7) continue;
        if(!strstr(path,".so")) continue;
        std::string b=base_of(path);
        if(b.empty()) continue;
        auto it=out.find(b);
        if(it==out.end()) out[b]=std::make_pair(st,std::string(path));
        else if(off==0) out[b]=std::make_pair(st,std::string(path));
    }
    fclose(f);
}

static uintptr_t rsym(std::vector<resolver>& R,const char* n){
    for(size_t i=0;i<R.size();i++){ uint64_t v=R[i].find(n); if(v) return v; }
    return 0;
}

#define R_AARCH64_ABS64   257
#define R_AARCH64_GLOB_DAT 1025
#define R_AARCH64_JUMP_SLOT 1026
#define R_AARCH64_RELATIVE 1027
#define R_AARCH64_IRELATIVE 1032

int main(int argc,char** argv){
    if(argc<3){ logf("usage: %s <pid> <lib.so>\n",argv[0]); return 1; }
    int pid=atoi(argv[1]);
    const char* path=argv[2];
    logf("[*] inj pid=%d lib=%s\n",pid,path);

    int fd=open(path,O_RDONLY);
    if(fd<0){ logf("[-] open lib fail\n"); return 1; }
    off_t fsz=lseek(fd,0,SEEK_END); lseek(fd,0,SEEK_SET);
    if(fsz<=0||fsz>64*1024*1024){ logf("[-] lib size bad\n"); close(fd); return 1; }
    std::vector<uint8_t> lib((size_t)fsz);
    if(read(fd,lib.data(),(size_t)fsz)!=(ssize_t)fsz){ close(fd); return 1; }
    close(fd);

    Elf64_Ehdr* eh=(Elf64_Ehdr*)lib.data();
    if(memcmp(eh->e_ident,"\x7f""ELF",4)||eh->e_ident[EI_CLASS]!=ELFCLASS64||eh->e_machine!=EM_AARCH64){
        logf("[-] not aarch64 elf\n"); return 1;
    }

    std::vector<seg> S;
    uint64_t min_v=~0ull,max_e=0;
    Elf64_Phdr* ph=(Elf64_Phdr*)(lib.data()+eh->e_phoff);
    for(int i=0;i<eh->e_phnum;i++){
        if(ph[i].p_type==PT_LOAD){
            S.push_back({ph[i].p_vaddr,ph[i].p_filesz,ph[i].p_memsz,ph[i].p_offset,ph[i].p_align,ph[i].p_flags});
            if(ph[i].p_vaddr<min_v) min_v=ph[i].p_vaddr;
            uint64_t e=ph[i].p_vaddr+ph[i].p_memsz;
            if(e>max_e) max_e=e;
        }
    }
    if(S.empty()){ logf("[-] no load segs\n"); return 1; }
    uint64_t msize=alu(max_e-min_v);
    logf("[+] module size 0x%llx\n",(unsigned long long)msize);

    uint64_t dyn_va=0,dyn_sz=0;
    for(int i=0;i<eh->e_phnum;i++) if(ph[i].p_type==PT_DYNAMIC){ dyn_va=ph[i].p_vaddr; dyn_sz=ph[i].p_memsz; break; }
    if(!dyn_va){ logf("[-] no dynamic\n"); return 1; }

    uint64_t symtab=0,strtab=0,strsz=0,nsym=0;
    uint64_t rela=0,relasz=0,relaent=24;
    uint64_t rel=0,relsz=0,relent=16;
    uint64_t jmprel=0,pltrelsz=0,pltrel=0;
    uint64_t init_fn=0,init_ar=0,init_arsz=0;
    {
        Elf64_Dyn* D=(Elf64_Dyn*)(lib.data()+v2o(S,dyn_va));
        size_t dn=dyn_sz/16;
        for(size_t i=0;i<dn;i++){
            switch(D[i].d_tag){
                case DT_SYMTAB: symtab=D[i].d_un.d_ptr; break;
                case DT_STRTAB: strtab=D[i].d_un.d_ptr; break;
                case DT_STRSZ: strsz=D[i].d_un.d_val; break;
                case DT_HASH:{ uint32_t* h=(uint32_t*)(lib.data()+v2o(S,D[i].d_un.d_ptr)); nsym=h[1]; break; }
                case DT_GNU_HASH:{
                    uint32_t* g=(uint32_t*)(lib.data()+v2o(S,D[i].d_un.d_ptr));
                    uint32_t nb=g[0],so=g[1],bs=g[2];
                    uint32_t* buckets=(uint32_t*)((uint64_t)g+16+bs*8);
                    uint32_t* chain=buckets+nb;
                    uint32_t mx=0;
                    for(uint32_t b=0;b<nb;b++){
                        uint32_t ix=buckets[b];
                        if(!ix) continue;
                        while(1){ if(ix>mx) mx=ix; uint32_t ch=chain[ix-so]; ix++; if(ch&1) break; }
                    }
                    nsym=mx+1;
                    break;
                }
                case DT_RELA: rela=D[i].d_un.d_ptr; break;
                case DT_RELASZ: relasz=D[i].d_un.d_val; break;
                case DT_RELAENT: relaent=D[i].d_un.d_val; break;
                case DT_REL: rel=D[i].d_un.d_ptr; break;
                case DT_RELSZ: relsz=D[i].d_un.d_val; break;
                case DT_RELENT: relent=D[i].d_un.d_val; break;
                case DT_JMPREL: jmprel=D[i].d_un.d_ptr; break;
                case DT_PLTRELSZ: pltrelsz=D[i].d_un.d_val; break;
                case DT_PLTREL: pltrel=D[i].d_un.d_val; break;
                case DT_INIT: init_fn=D[i].d_un.d_ptr; break;
                case DT_INIT_ARRAY: init_ar=D[i].d_un.d_ptr; break;
                case DT_INIT_ARRAYSZ: init_arsz=D[i].d_un.d_val; break;
            }
        }
    }
    if(!relaent) relaent=24;
    if(!relent) relent=16;

    char memp[64]; snprintf(memp,64,"/proc/%d/mem",pid);
    mfd=open(memp,O_RDWR);
    if(mfd<0){ logf("[-] open /proc/%d/mem fail %s\n",pid,strerror(errno)); return 1; }
    logf("[+] mem ok\n");

    std::map<std::string,std::pair<uint64_t,std::string>> rl;
    collect_libs(pid,rl);
    logf("[+] remote libs %zu\n",rl.size());
    const char* want[]={"libc.so","libdl.so","libm.so","liblog.so","libandroid.so","libEGL.so","libGLESv3.so","libGLESv2.so","libunity.so",0};
    std::vector<resolver> res;
    for(const char** w=want;*w;w++){
        auto it=rl.find(*w);
        if(it==rl.end()){ logf("[-] no remote %s\n",*w); continue; }
        resolver r;
        if(r.load(it->second.second,it->second.first)){
            logf("[+] resolver %s base=%llx\n",*w,(unsigned long long)it->second.first);
            res.push_back(std::move(r));
        }
    }

    uintptr_t mmap_a=rsym(res,"mmap");
    uintptr_t mprotect_a=rsym(res,"mprotect");
    if(!mmap_a||!mprotect_a){ logf("[-] no mmap/mprotect\n"); return 1; }

    uintptr_t base=rcall(pid,mmap_a,6,0,msize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,(uintptr_t)-1,0);
    if(base==(uintptr_t)-1||base==0){ logf("[-] remote mmap fail\n"); return 1; }
    logf("[+] mapped at %llx\n",(unsigned long long)base);

    // write segments
    for(size_t i=0;i<S.size();i++){
        uint64_t a=base+(S[i].va-min_v);
        if(S[i].fsz) rwrite(a,lib.data()+S[i].off,S[i].fsz);
        if(S[i].msz>S[i].fsz){
            static uint8_t z[4096]={0};
            uint64_t left=S[i].msz-S[i].fsz;
            uint64_t p=a+S[i].fsz;
            while(left){
                uint64_t c=left>sizeof(z)?sizeof(z):left;
                rwrite(p,z,c);
                p+=c; left-=c;
            }
        }
    }

    // relocations
    auto w64=[&](uint64_t a,uint64_t v){ rwrite(a,&v,8); };
    auto w32=[&](uint64_t a,uint32_t v){ rwrite(a,&v,4); };
    auto getS=[&](uint32_t si,uint64_t* out)->bool{
        if(!si){ *out=0; return false; }
        uint64_t so=v2o(S,symtab)+(uint64_t)si*24;
        if(so+24>lib.size()) return false;
        Elf64_Sym* s=(Elf64_Sym*)(lib.data()+so);
        if(s->st_shndx!=SHN_UNDEF&&s->st_value){ *out=base+(s->st_value-min_v); return true; }
        if(!s->st_name) return false;
        const char* nm=(const char*)(lib.data()+v2o(S,strtab)+s->st_name);
        for(size_t i=0;i<res.size();i++){ uint64_t v=res[i].find(nm); if(v){ *out=v; return true; } }
        return false;
    };
    auto do_rela=[&](uint64_t r_offset,uint64_t r_info,int64_t r_addend){
        uint32_t ty=(uint32_t)r_info;
        uint32_t si=(uint32_t)(r_info>>32);
        uint64_t a=base+(r_offset-min_v);
        uint64_t Sv=0;
        switch(ty){
            case R_AARCH64_RELATIVE: w64(a,base+(uint64_t)r_addend); break;
            case R_AARCH64_ABS64: if(getS(si,&Sv)) w64(a,Sv+(uint64_t)r_addend); break;
            case R_AARCH64_GLOB_DAT:
            case R_AARCH64_JUMP_SLOT: if(getS(si,&Sv)) w64(a,Sv); break;
            default: break;
        }
    };
    size_t nr=relaent?relasz/relaent:0;
    for(size_t i=0;i<nr;i++){
        Elf64_Rela* r=(Elf64_Rela*)(lib.data()+v2o(S,rela)+i*24);
        do_rela(r->r_offset,r->r_info,r->r_addend);
    }
    if(pltrel==DT_RELA){
        size_t nj=pltrelsz/24;
        for(size_t j=0;j<nj;j++){
            Elf64_Rela* r=(Elf64_Rela*)(lib.data()+v2o(S,jmprel)+j*24);
            do_rela(r->r_offset,r->r_info,r->r_addend);
        }
    }
    size_t nr2=relent?relsz/relent:0;
    for(size_t i=0;i<nr2;i++){
        Elf64_Rel* r=(Elf64_Rel*)(lib.data()+v2o(S,rel)+i*16);
        do_rela(r->r_offset,r->r_info,0);
    }
    logf("[+] relocs done\n");

    // mprotect segments
    for(size_t i=0;i<S.size();i++){
        int prot=0;
        if(S[i].fl&PF_R) prot|=PROT_READ;
        if(S[i].fl&PF_W) prot|=PROT_WRITE;
        if(S[i].fl&PF_X) prot|=PROT_EXEC;
        uint64_t a=base+(S[i].va-min_v);
        if(prot&&prot!=(PROT_READ|PROT_WRITE)){
            uintptr_t mr=rcall(pid,mprotect_a,3,ald(a),alu(S[i].msz),prot);
            logf("[+] mprotect 0x%llx size 0x%llx prot 0x%x -> %llx\n",
                (unsigned long long)ald(a),(unsigned long long)alu(S[i].msz),prot,(unsigned long long)mr);
        }
    }

    // run init
    if(init_fn){
        logf("[+] DT_INIT\n");
        rcall(pid,base+(init_fn-min_v),1,base);
    }
    if(init_ar&&init_arsz){
        size_t n=init_arsz/8;
        for(size_t i=0;i<n;i++){
            uint64_t fp=0;
            if(rread(base+(init_ar-min_v)+i*8,&fp,8)==8&&fp){
                logf("[+] init_array[%zu]=%llx\n",i,(unsigned long long)fp);
                rcall(pid,fp,1,base);
            }
        }
    }

    // payload_entry
    uintptr_t entry=0;
    {
        uint64_t so=v2o(S,symtab);
        size_t lim=nsym?nsym:(strsz);
        for(size_t i=0;i<lim;i++){
            Elf64_Sym* s=(Elf64_Sym*)(lib.data()+so+i*24);
            if(!s->st_name||!s->st_shndx) continue;
            const char* nm=(const char*)(lib.data()+v2o(S,strtab)+s->st_name);
            if(!strcmp(nm,"payload_entry")){ entry=base+(s->st_value-min_v); break; }
        }
    }
    if(entry){
        logf("[+] payload_entry %llx\n",(unsigned long long)entry);
        rcall(pid,entry,1,base);
        logf("[+] injected\n");
    }else{
        logf("[-] payload_entry not found\n");
    }

    close(mfd);
    return 0;
}
