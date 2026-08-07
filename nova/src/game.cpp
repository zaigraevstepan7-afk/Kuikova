#include "game.hpp"
#include "offsets.hpp"
#include "mem.hpp"
#include "module_base.hpp"
#include "chams.hpp"

namespace {

uintptr_t chain_manager_from_klass(uintptr_t klass) {
    if (!klass) return 0;
    const uintptr_t sf = mem::read_ptr(klass + off::mgr::kStaticFields);
    if (!sf) return 0;
    uintptr_t inst = mem::read_ptr(sf + off::mgr::kPtr2);
    if (inst) {
        const uintptr_t via0 = mem::read_ptr(inst + off::mgr::kPtr3);
        if (via0) return via0;
        return inst;
    }
    return mem::read_ptr(sf + off::mgr::kPtr3);
}

uintptr_t resolve_manager(uintptr_t il2cpp) {
    const uintptr_t ti_loc = il2cpp + off::kPlayerManagerTI;
    const uintptr_t klass = mem::read_ptr(ti_loc);
    if (klass) {
        if (uintptr_t inst = chain_manager_from_klass(klass))
            return inst;
    }
    return chain_manager_from_klass(ti_loc);
}

} // namespace

bool game_init() { return true; }

void game_tick(GameState& st) {
    st.frame++;

    if (!st.il2cpp) {
        st.il2cpp = mods::resolve_il2cpp();
        if (!st.il2cpp) {
            st.status = "wait";
            st.ready = false;
            return;
        }
    }
    if (!st.unity) st.unity = mods::resolve_unity();

    // Hook once — cheap after first success
    chams_install();

    // /proc/self/mem manager resolve is expensive — refresh rarely
    if (!st.manager || (st.frame % 90) == 0) {
        st.manager = resolve_manager(st.il2cpp);
    }
    if (!st.manager) {
        st.status = "lobby";
        st.ready = false;
        st.local = 0;
        return;
    }

    if (!st.local || (st.frame % 90) == 0) {
        st.local = mem::read_ptr(st.manager + off::mgr::kLocal);
        if (!st.local) st.local = mem::read_ptr(st.manager + 0x68);
    }

    st.ready = true;
    if (!st.local) st.status = "nolocal";
    else if (chams_hook_ok() == 0) st.status = chams_status();
    else st.status = "ok";
}
