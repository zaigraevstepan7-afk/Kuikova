# Halalium decompile index (2-pass)

Each function: **Pass A** = full Capstone disasm, **Pass B** = field map + mov/movk RVAs + ADRP+ADD strings.

- [egl_install](egl_install.md) @ `0x1d84cc` — fields=1, RVAs=[`0x8e7c40c`, `0x8e0085c`, `0x79fe5e0`, `0x147e970`], strings=4
- [egl_callback](egl_callback.md) @ `0x1d76f0` — fields=4, RVAs=[—], strings=0
- [input_consume_cb](input_consume_cb.md) @ `0x1d760c` — fields=1, RVAs=[—], strings=0
- [Update_Halalium_Hooks](Update_Halalium_Hooks.md) @ `0x1d7a10` — fields=21, RVAs=[`0x8c69cb0`, `0x8e8ae24`, `0x5fb7bc0`], strings=2
- [LateUpdate_cb](LateUpdate_cb.md) @ `0x1d7ec4` — fields=14, RVAs=[`0x8c7be04`, `0x5caafc4`, `0x5deada4`], strings=1
- [secondary_hook_cb](secondary_hook_cb.md) @ `0x1d81fc` — fields=8, RVAs=[`0x5ffaefc`, `0x6009294`], strings=0
- [tertiary_hook_cb](tertiary_hook_cb.md) @ `0x1d8404` — fields=3, RVAs=[—], strings=2
- [extraA_cb](extraA_cb.md) @ `0x1d82a0` — fields=4, RVAs=[—], strings=0
- [extraB_cb](extraB_cb.md) @ `0x1d83cc` — fields=1, RVAs=[—], strings=0
- [Bypass_getrr](Bypass_getrr.md) @ `0x1d90b8` — fields=1, RVAs=[—], strings=3
- [libunity_base_resolve](libunity_base_resolve.md) @ `0x1d6afc` — fields=15, RVAs=[—], strings=1
- [menu_body](menu_body.md) @ `0x1db874` — fields=15, RVAs=[—], strings=52
- [wm_click_site](wm_click_site.md) @ `0x1db464` — fields=9, RVAs=[—], strings=1
- [track_hook_helper](track_hook_helper.md) @ `0x1d917c` — fields=1, RVAs=[—], strings=0
