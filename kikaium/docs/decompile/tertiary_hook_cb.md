# tertiary_hook_cb

- VA: `0x1d8404`
- Size scanned: `0x100`
- Pass A instructions: 64
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0xc` | unknown | 1 |
| `0x10` | unknown | 1 |
| `0x14` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

- `libEGL.so`
- `eglSwapBuffers`

## Pass B — calls (first 40)

- `0x1d84f0` → `0x26c550` (bl)

## Pass A — full disasm

```
001d8404: adrp     x8, #0x279000
001d8408: ldrb     w8, [x8, #0x5a1]
001d840c: cbz      w8, #0x1d84c0
001d8410: mov      w8, #0x447a0000
001d8414: fmov     s1, w8
001d8418: fcmp     s0, s1
001d841c: b.ne     #0x1d84c0
001d8420: mov      w8, #0x6010
001d8424: movk     w8, #0x6000, lsl #16
001d8428: cmp      w2, w8
001d842c: b.ne     #0x1d84c0
001d8430: cbz      x0, #0x1d84c0
001d8434: adrp     x8, #0x279000
001d8438: ldrb     w8, [x8, #0x5cc]
001d843c: cbz      w8, #0x1d84c0
001d8440: adrp     x8, #0x279000
001d8444: ldr      w8, [x8, #0x5e8]
001d8448: cbnz     w8, #0x1d84c0
001d844c: nop      
001d8450: adr      x8, #0x279490
001d8454: ldr      d2, [x0]
001d8458: ldr      d1, [x8]
001d845c: ldr      s4, [x0, #8]
001d8460: fsub     v1.2s, v1.2s, v2.2s
001d8464: fmul     v2.2s, v1.2s, v1.2s
001d8468: mov      s3, v2.s[1]
001d846c: ldr      s2, [x8, #8]
001d8470: adrp     x8, #0x37000
001d8474: fsub     s2, s2, s4
001d8478: ldr      s4, [x8, #0xf14]
001d847c: fmadd    s3, s1, s1, s3
001d8480: fmadd    s3, s2, s2, s3
001d8484: fsqrt    s3, s3
001d8488: fcmp     s3, s4
001d848c: b.le     #0x1d84c0
001d8490: adrp     x8, #0x37000
001d8494: ldr      s4, [x8, #0xf3c]
001d8498: fcmp     s3, s4
001d849c: b.le     #0x1d84b0
001d84a0: dup      v4.2s, v3.s[0]
001d84a4: fdiv     s2, s2, s3
001d84a8: fdiv     v1.2s, v1.2s, v4.2s
001d84ac: b        #0x1d84b8
001d84b0: movi     d1, #0000000000000000
001d84b4: movi     d2, #0000000000000000
001d84b8: stur     d1, [x0, #0xc]
001d84bc: str      s2, [x0, #0x14]
001d84c0: adrp     x8, #0x279000
001d84c4: ldr      x4, [x8, #0x520]
001d84c8: br       x4
001d84cc: stp      x29, x30, [sp, #-0x40]!
001d84d0: str      x23, [sp, #0x10]
001d84d4: stp      x22, x21, [sp, #0x20]
001d84d8: stp      x20, x19, [sp, #0x30]
001d84dc: mov      x29, sp
001d84e0: adrp     x0, #0x36000
001d84e4: add      x0, x0, #0x373
001d84e8: adrp     x1, #0x35000
001d84ec: add      x1, x1, #0x75b
001d84f0: bl       #0x26c550
001d84f4: cbz      x0, #0x1d850c
001d84f8: nop      
001d84fc: adr      x1, #0x1d76f0
001d8500: nop      
```
