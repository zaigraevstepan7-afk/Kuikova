# egl_callback

- VA: `0x1d76f0`
- Size scanned: `0x320`
- Pass A instructions: 200
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 1 |
| `0x18` | unknown | 1 |
| `0x20` | unknown | 4 |
| `0x28` | players_list / camera holder | 3 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

_none_

## Pass B — calls (first 40)

- `0x1d7730` → `0x26c4f0` (bl)
- `0x1d773c` → `0x1ef77c` (bl)
- `0x1d7748` → `0x22e93c` (bl)
- `0x1d774c` → `0x1f2744` (bl)
- `0x1d7754` → `0x1daaec` (bl)
- `0x1d7768` → `0x1f2744` (bl)
- `0x1d7780` → `0x26c500` (bl)
- `0x1d7794` → `0x26c500` (bl)
- `0x1d77b8` → `0x26c510` (bl)
- `0x1d781c` → `0x26c4b0` (bl)
- `0x1d7834` → `0x1ebd9c` (bl)
- `0x1d7848` → `0x1ebf60` (bl)
- `0x1d7854` → `0x26c4e0` (bl)
- `0x1d7858` → `0x22eaf8` (bl)
- `0x1d785c` → `0x1f3570` (bl)
- `0x1d7880` → `0x1da768` (bl)
- `0x1d7890` → `0x1e955c` (bl)
- `0x1d78d4` → `0x1ed030` (bl)
- `0x1d78dc` → `0x1f288c` (bl)
- `0x1d78f8` → `0x208f88` (bl)
- `0x1d7908` → `0x1f288c` (bl)
- `0x1d794c` → `0x208758` (bl)
- `0x1d7974` → `0x208758` (bl)
- `0x1d7978` → `0x1dac28` (bl)
- `0x1d797c` → `0x1db864` (bl)
- `0x1d7980` → `0x1fba6c` (bl)
- `0x1d7994` → `0x26c520` (bl)
- `0x1d7998` → `0x1f2764` (bl)
- `0x1d799c` → `0x22ef18` (bl)
- `0x1d79b0` → `x8` (blr)
- `0x1d79ec` → `0x26c4e0` (bl)
- `0x1d7a04` → `0x26841c` (bl)
- `0x1d7a08` → `0x26c3b0` (bl)

## Pass A — full disasm

```
001d76f0: sub      sp, sp, #0x80
001d76f4: stp      d9, d8, [sp, #0x30]
001d76f8: stp      x29, x30, [sp, #0x40]
001d76fc: stp      x24, x23, [sp, #0x50]
001d7700: stp      x22, x21, [sp, #0x60]
001d7704: stp      x20, x19, [sp, #0x70]
001d7708: add      x29, sp, #0x40
001d770c: mrs      x22, tpidr_el0
001d7710: nop      
001d7714: adr      x21, #0x27965c
001d7718: ldr      x8, [x22, #0x28]
001d771c: mov      x19, x1
001d7720: mov      x20, x0
001d7724: stur     x8, [x29, #-0x18]
001d7728: ldarb    w8, [x21]
001d772c: tbnz     w8, #0, #0x1d7760
001d7730: bl       #0x26c4f0
001d7734: cbz      x0, #0x1d7760
001d7738: mov      x0, xzr
001d773c: bl       #0x1ef77c
001d7740: adrp     x0, #0x2e000
001d7744: add      x0, x0, #0xebe
001d7748: bl       #0x22e93c
001d774c: bl       #0x1f2744
001d7750: str      xzr, [x0, #0x20]
001d7754: bl       #0x1daaec
001d7758: mov      w8, #1
001d775c: stlrb    w8, [x21]
001d7760: ldarb    w8, [x21]
001d7764: tbz      w8, #0, #0x1d79a0
001d7768: bl       #0x1f2744
001d776c: mov      x21, x0
001d7770: sub      x3, x29, #0x1c
001d7774: mov      x0, x20
001d7778: mov      x1, x19
001d777c: mov      w2, #0x3057
001d7780: bl       #0x26c500
001d7784: add      x3, sp, #0x20
001d7788: mov      x0, x20
001d778c: mov      x1, x19
001d7790: mov      w2, #0x3056
001d7794: bl       #0x26c500
001d7798: ldur     s0, [x29, #-0x1c]
001d779c: ldr      s1, [sp, #0x20]
001d77a0: add      x1, sp, #0x10
001d77a4: mov      w0, #1
001d77a8: stp      xzr, xzr, [sp, #0x10]
001d77ac: scvtf    s0, s0
001d77b0: scvtf    s1, s1
001d77b4: stp      s0, s1, [x21, #8]
001d77b8: bl       #0x26c510
001d77bc: nop      
001d77c0: adr      x8, #0x279588
001d77c4: ldp      x9, x10, [x8]
001d77c8: orr      x11, x9, x10
001d77cc: cbz      x11, #0x1d780c
001d77d0: ldp      x11, x12, [sp, #0x10]
001d77d4: sub      x9, x11, x9
001d77d8: sub      x10, x12, x10
001d77dc: scvtf    s0, x9
001d77e0: scvtf    s1, x10
001d77e4: adrp     x9, #0x37000
001d77e8: ldr      s2, [x9, #0xf58]
001d77ec: adrp     x9, #0x37000
001d77f0: fmadd    s0, s1, s2, s0
001d77f4: fmov     s1, #0.50000000
001d77f8: fminnm   s1, s0, s1
001d77fc: fcmp     s0, #0.0
001d7800: ldr      s0, [x9, #0xea0]
001d7804: fcsel    s0, s1, s0, gt
001d7808: str      s0, [x21, #0x18]
001d780c: ldr      q0, [sp, #0x10]
001d7810: nop      
001d7814: adr      x0, #0x27954c
001d7818: str      q0, [x8]
001d781c: bl       #0x26c4b0
001d7820: adrp     x8, #0x279000
001d7824: adrp     x9, #0x279000
001d7828: ldr      s0, [x8, #0x650]
001d782c: ldr      s1, [x9, #0x654]
001d7830: mov      x0, x21
001d7834: bl       #0x1ebd9c
001d7838: adrp     x8, #0x279000
001d783c: ldrb     w2, [x8, #0x658]
001d7840: mov      x0, x21
001d7844: mov      w1, wzr
001d7848: bl       #0x1ebf60
001d784c: nop      
001d7850: adr      x0, #0x27954c
001d7854: bl       #0x26c4e0
001d7858: bl       #0x22eaf8
001d785c: bl       #0x1f3570
001d7860: adrp     x10, #0x279000
001d7864: ldr      w8, [sp, #0x20]
001d7868: ldur     w9, [x29, #-0x1c]
001d786c: ldr      x0, [x10, #0x510]
001d7870: adrp     x23, #0x279000
001d7874: adrp     x24, #0x279000
001d7878: str      w8, [x23, #0x598]
001d787c: str      w9, [x24, #0x59c]
001d7880: bl       #0x1da768
001d7884: adrp     x8, #0x279000
001d7888: ldrb     w8, [x8, #0x5a0]
001d788c: cbz      w8, #0x1d7894
001d7890: bl       #0x1e955c
001d7894: adrp     x8, #0x279000
001d7898: ldrb     w8, [x8, #0x5a1]
001d789c: cbz      w8, #0x1d78fc
001d78a0: adrp     x8, #0x279000
001d78a4: ldrb     w8, [x8, #0x5a2]
001d78a8: cbz      w8, #0x1d78fc
001d78ac: ldr      s0, [x24, #0x59c]
001d78b0: ldr      s1, [x23, #0x598]
001d78b4: fmov     s2, #0.50000000
001d78b8: nop      
001d78bc: adr      x0, #0x278ff8
001d78c0: scvtf    s0, s0
001d78c4: scvtf    s1, s1
001d78c8: fmul     s0, s0, s2
001d78cc: fmul     s1, s1, s2
001d78d0: stp      s0, s1, [sp, #0x10]
001d78d4: bl       #0x1ed030
001d78d8: mov      w21, w0
001d78dc: bl       #0x1f288c
001d78e0: adrp     x8, #0x279000
001d78e4: fmov     s1, #2.00000000
001d78e8: add      x1, sp, #0x10
001d78ec: ldr      s0, [x8, #0x5a4]
001d78f0: mov      w2, w21
001d78f4: mov      w3, #0x40
001d78f8: bl       #0x208f88
001d78fc: adrp     x8, #0x279000
001d7900: ldrb     w8, [x8, #0x548]
001d7904: cbz      w8, #0x1d7978
001d7908: bl       #0x1f288c
001d790c: ldr      s0, [x24, #0x59c]
001d7910: ldr      s1, [x23, #0x598]
001d7914: fmov     s2, #0.50000000
001d7918: add      x1, sp, #0x10
001d791c: add      x2, sp, #8
001d7920: mov      w3, #-0xffff01
001d7924: scvtf    s0, s0
001d7928: scvtf    s1, s1
001d792c: mov      x21, x0
001d7930: fmul     s8, s0, s2
001d7934: fmul     s9, s1, s2
001d7938: fsub     s0, s8, s9
001d793c: fadd     s1, s8, s9
001d7940: stp      s0, s9, [sp, #0x10]
001d7944: fmov     s0, #2.00000000
001d7948: stp      s1, s9, [sp, #8]
001d794c: bl       #0x208758
001d7950: fsub     s0, s9, s9
001d7954: fadd     s1, s9, s9
001d7958: add      x1, sp, #0x10
001d795c: add      x2, sp, #8
001d7960: mov      x0, x21
001d7964: mov      w3, #-0xffff01
001d7968: stp      s8, s0, [sp, #0x10]
001d796c: fmov     s0, #2.00000000
001d7970: stp      s8, s1, [sp, #8]
001d7974: bl       #0x208758
001d7978: bl       #0x1dac28
001d797c: bl       #0x1db864
001d7980: bl       #0x1fba6c
001d7984: ldur     w2, [x29, #-0x1c]
001d7988: ldr      w3, [sp, #0x20]
001d798c: mov      w0, wzr
001d7990: mov      w1, wzr
001d7994: bl       #0x26c520
001d7998: bl       #0x1f2764
001d799c: bl       #0x22ef18
001d79a0: adrp     x8, #0x279000
001d79a4: mov      x0, x20
001d79a8: mov      x1, x19
001d79ac: ldr      x8, [x8, #0x578]
001d79b0: blr      x8
001d79b4: ldr      x8, [x22, #0x28]
001d79b8: ldur     x9, [x29, #-0x18]
001d79bc: cmp      x8, x9
001d79c0: b.ne     #0x1d7a08
001d79c4: ldp      x20, x19, [sp, #0x70]
001d79c8: ldp      x22, x21, [sp, #0x60]
001d79cc: ldp      x24, x23, [sp, #0x50]
001d79d0: ldp      x29, x30, [sp, #0x40]
001d79d4: ldp      d9, d8, [sp, #0x30]
001d79d8: add      sp, sp, #0x80
001d79dc: ret      
001d79e0: mov      x19, x0
001d79e4: nop      
001d79e8: adr      x0, #0x27954c
001d79ec: bl       #0x26c4e0
001d79f0: ldr      x8, [x22, #0x28]
001d79f4: ldur     x9, [x29, #-0x18]
001d79f8: cmp      x8, x9
001d79fc: b.ne     #0x1d7a08
001d7a00: mov      x0, x19
001d7a04: bl       #0x26841c
001d7a08: bl       #0x26c3b0
001d7a0c: str      d8, [sp, #-0x60]!
```
