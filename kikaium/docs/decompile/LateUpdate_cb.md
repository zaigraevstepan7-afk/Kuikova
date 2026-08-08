# LateUpdate_cb

- VA: `0x1d7ec4`
- Size scanned: `0x340`
- Pass A instructions: 208
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x8` | unknown | 3 |
| `0x10` | unknown | 3 |
| `0x14` | unknown | 1 |
| `0x18` | unknown | 3 |
| `0x1c` | unknown | 1 |
| `0x20` | unknown | 1 |
| `0x28` | players_list / camera holder | 2 |
| `0x38` | unknown | 1 |
| `0x40` | unknown | 1 |
| `0x98` | Player.movement | 1 |
| `0xc0` | unknown | 1 |
| `0x150` | unknown | 1 |
| `0x160` | Player.photon_player | 2 |
| `0x190` | unknown | 1 |
| `0x2b0` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

- `w24` = `0x8c7be04` @ `0x1d8078`
- `w28` = `0x5caafc4` @ `0x1d80b4`
- `w9` = `0x5deada4` @ `0x1d817c`

## Pass B — strings (ADRP+ADD resolved only)

- `InstantiateViaServer`

## Pass B — calls (first 40)

- `0x1d7f08` → `x8` (blr)
- `0x1d7f60` → `0x1e0bcc` (bl)
- `0x1d7f78` → `0x1d88fc` (bl)
- `0x1d7f8c` → `0x1d8ee4` (bl)
- `0x1d7f90` → `0x1d8f2c` (bl)
- `0x1d7f9c` → `0x1d8f48` (bl)
- `0x1d7fb4` → `0x1d8f74` (bl)
- `0x1d7fc4` → `0x1d8f08` (bl)
- `0x1d7fdc` → `0x1d9c7c` (bl)
- `0x1d800c` → `0x1d8858` (bl)
- `0x1d8020` → `0x1d8858` (bl)
- `0x1d8034` → `0x1d8858` (bl)
- `0x1d8084` → `x8` (blr)
- `0x1d80d0` → `x8` (blr)
- `0x1d8114` → `x28` (blr)
- `0x1d8124` → `x28` (blr)
- `0x1d8134` → `x28` (blr)
- `0x1d8144` → `x28` (blr)
- `0x1d8154` → `x28` (blr)
- `0x1d8188` → `x8` (blr)
- `0x1d81b0` → `x8` (blr)
- `0x1d81b8` → `0x26c3b0` (bl)

## Pass A — full disasm

```
001d7ec4: sub      sp, sp, #0xa0
001d7ec8: stp      d9, d8, [sp, #0x30]
001d7ecc: stp      x29, x30, [sp, #0x40]
001d7ed0: stp      x28, x27, [sp, #0x50]
001d7ed4: stp      x26, x25, [sp, #0x60]
001d7ed8: stp      x24, x23, [sp, #0x70]
001d7edc: stp      x22, x21, [sp, #0x80]
001d7ee0: stp      x20, x19, [sp, #0x90]
001d7ee4: add      x29, sp, #0x40
001d7ee8: mrs      x26, tpidr_el0
001d7eec: mov      x19, x0
001d7ef0: ldr      x8, [x26, #0x28]
001d7ef4: stur     x8, [x29, #-0x18]
001d7ef8: adrp     x8, #0x279000
001d7efc: ldr      x8, [x8, #0x5f0]
001d7f00: cbz      x8, #0x1d7f0c
001d7f04: mov      x0, x19
001d7f08: blr      x8
001d7f0c: adrp     x8, #0x279000
001d7f10: ldr      x8, [x8, #0x4f8]
001d7f14: ldr      x8, [x8, #8]
001d7f18: cmp      x8, x19
001d7f1c: b.eq     #0x1d7f54
001d7f20: ldr      x8, [x26, #0x28]
001d7f24: ldur     x9, [x29, #-0x18]
001d7f28: cmp      x8, x9
001d7f2c: b.ne     #0x1d81b8
001d7f30: ldp      x20, x19, [sp, #0x90]
001d7f34: ldp      x22, x21, [sp, #0x80]
001d7f38: ldp      x24, x23, [sp, #0x70]
001d7f3c: ldp      x26, x25, [sp, #0x60]
001d7f40: ldp      x28, x27, [sp, #0x50]
001d7f44: ldp      x29, x30, [sp, #0x40]
001d7f48: ldp      d9, d8, [sp, #0x30]
001d7f4c: add      sp, sp, #0xa0
001d7f50: ret      
001d7f54: adrp     x8, #0x279000
001d7f58: mov      x1, x19
001d7f5c: ldr      x0, [x8, #0x500]
001d7f60: bl       #0x1e0bcc
001d7f64: adrp     x8, #0x279000
001d7f68: ldrb     w8, [x8, #0x5f8]
001d7f6c: cbz      w8, #0x1d7fbc
001d7f70: ldr      x0, [x19, #0x160]
001d7f74: cbz      x0, #0x1d7fc8
001d7f78: bl       #0x1d88fc
001d7f7c: cmp      w0, #1
001d7f80: b.lt     #0x1d7fc8
001d7f84: mov      x0, x19
001d7f88: mov      x1, x19
001d7f8c: bl       #0x1d8ee4
001d7f90: bl       #0x1d8f2c
001d7f94: cbz      x0, #0x1d7fc8
001d7f98: mov      x1, x0
001d7f9c: bl       #0x1d8f48
001d7fa0: cbz      x0, #0x1d7fc8
001d7fa4: movi     d0, #0000000000000000
001d7fa8: movi     d1, #0000000000000000
001d7fac: mov      x1, x0
001d7fb0: fmov     s2, #-2.00000000
001d7fb4: bl       #0x1d8f74
001d7fb8: b        #0x1d7fc8
001d7fbc: mov      x0, x19
001d7fc0: mov      x1, x19
001d7fc4: bl       #0x1d8f08
001d7fc8: adrp     x20, #0x279000
001d7fcc: ldrb     w8, [x20, #0x5f9]
001d7fd0: cbz      w8, #0x1d7fe4
001d7fd4: adrp     x8, #0x279000
001d7fd8: ldr      x0, [x8, #0x528]
001d7fdc: bl       #0x1d9c7c
001d7fe0: strb     wzr, [x20, #0x5f9]
001d7fe4: adrp     x8, #0x279000
001d7fe8: ldrb     w8, [x8, #0x5fa]
001d7fec: cbz      w8, #0x1d7f20
001d7ff0: ldr      x21, [x19, #0x160]
001d7ff4: cbz      x21, #0x1d7f20
001d7ff8: ldr      x8, [x21, #0x38]
001d7ffc: cbz      x8, #0x1d7f20
001d8000: adrp     x1, #0x37000
001d8004: add      x1, x1, #0xf64
001d8008: mov      x0, x21
001d800c: bl       #0x1d8858
001d8010: mov      x22, x0
001d8014: adrp     x1, #0x37000
001d8018: add      x1, x1, #0xf94
001d801c: mov      x0, x21
001d8020: bl       #0x1d8858
001d8024: mov      x20, x0
001d8028: adrp     x1, #0x37000
001d802c: add      x1, x1, #0xfbc
001d8030: mov      x0, x21
001d8034: bl       #0x1d8858
001d8038: cbz      x22, #0x1d804c
001d803c: ldrb     w8, [x22, #0x10]
001d8040: cmp      w8, #0
001d8044: cset     w21, ne
001d8048: b        #0x1d8050
001d804c: mov      w21, wzr
001d8050: movi     d9, #0000000000000000
001d8054: movi     d8, #0000000000000000
001d8058: cbz      x20, #0x1d8060
001d805c: ldr      d8, [x20, #0x10]
001d8060: cbz      x0, #0x1d8068
001d8064: ldr      d9, [x0, #0x10]
001d8068: adrp     x27, #0x279000
001d806c: mov      w24, #0xbe04
001d8070: mov      x0, xzr
001d8074: ldr      x8, [x27, #0x4f0]
001d8078: movk     w24, #0x8c7, lsl #16
001d807c: add      x8, x8, x24
001d8080: add      x8, x8, #0x470
001d8084: blr      x8
001d8088: fsub     d0, d0, d9
001d808c: fcmp     d0, d8
001d8090: csel     w8, wzr, w21, pl
001d8094: tbnz     w8, #0, #0x1d7f20
001d8098: ldr      x25, [x27, #0x4f0]
001d809c: mov      w8, #0x58d8
001d80a0: mov      w9, #0xb5c0
001d80a4: movk     w8, #0xb02, lsl #16
001d80a8: mov      w28, #0xafc4
001d80ac: movk     w9, #0xac4, lsl #16
001d80b0: add      x8, x25, x8
001d80b4: movk     w28, #0x5ca, lsl #16
001d80b8: ldr      x0, [x25, x9]
001d80bc: ldr      x23, [x8]
001d80c0: ldr      x22, [x8, #0x190]
001d80c4: ldr      x21, [x8, #0x98]
001d80c8: add      x8, x25, x28
001d80cc: mov      w1, #5
001d80d0: blr      x8
001d80d4: cbz      x0, #0x1d7f20
001d80d8: mov      w8, #0x64
001d80dc: mov      w9, #1
001d80e0: mov      x20, x0
001d80e4: stur     w8, [x29, #-0x1c]
001d80e8: add      x10, x25, x24
001d80ec: sub      x1, x29, #0x1c
001d80f0: str      w8, [sp, #0x20]
001d80f4: add      x8, x28, x25
001d80f8: mov      x0, x23
001d80fc: strb     w9, [sp, #0x1c]
001d8100: add      x28, x8, #0x98
001d8104: strb     w9, [sp, #0x18]
001d8108: mov      w9, #0x40a00000
001d810c: str      x10, [sp, #8]
001d8110: str      w9, [sp, #0x14]
001d8114: blr      x28
001d8118: mov      x24, x0
001d811c: add      x1, sp, #0x20
001d8120: mov      x0, x23
001d8124: blr      x28
001d8128: mov      x23, x0
001d812c: add      x1, sp, #0x1c
001d8130: mov      x0, x22
001d8134: blr      x28
001d8138: mov      x25, x0
001d813c: add      x1, sp, #0x18
001d8140: mov      x0, x22
001d8144: blr      x28
001d8148: mov      x22, x0
001d814c: add      x1, sp, #0x14
001d8150: mov      x0, x21
001d8154: blr      x28
001d8158: stp      x24, x23, [x20, #0x20]
001d815c: stp      x25, x22, [x20, #0x30]
001d8160: str      x0, [x20, #0x40]
001d8164: ldr      x8, [x27, #0x4f0]
001d8168: ldr      x19, [x19, #0x150]
001d816c: cbz      x8, #0x1d8194
001d8170: mov      w9, #0xada4
001d8174: adrp     x0, #0x35000
001d8178: add      x0, x0, #0x20c
001d817c: movk     w9, #0x5de, lsl #16
001d8180: mov      w1, #0x14
001d8184: add      x8, x8, x9
001d8188: blr      x8
001d818c: mov      x1, x0
001d8190: b        #0x1d8198
001d8194: mov      x1, xzr
001d8198: ldr      x8, [sp, #8]
001d819c: mov      x0, x19
001d81a0: mov      w2, wzr
001d81a4: mov      w3, #1
001d81a8: mov      x4, x20
001d81ac: mov      x5, xzr
001d81b0: blr      x8
001d81b4: b        #0x1d7f20
001d81b8: bl       #0x26c3b0
001d81bc: cbz      x0, #0x1d81f8
001d81c0: ldr      x8, [x0, #0x2b0]
001d81c4: cbz      x8, #0x1d81f8
001d81c8: ldr      x8, [x8, #0xc0]
001d81cc: cbz      x8, #0x1d81f8
001d81d0: ldr      x10, [x8, #0x18]
001d81d4: nop      
001d81d8: adr      x9, #0x1d7e14
001d81dc: cmp      x10, x9
001d81e0: b.eq     #0x1d81f8
001d81e4: adrp     x11, #0x279000
001d81e8: ldr      x12, [x11, #0x5e0]
001d81ec: cbnz     x12, #0x1d81f4
001d81f0: str      x10, [x11, #0x5e0]
001d81f4: str      x9, [x8, #0x18]
001d81f8: ret      
001d81fc: stp      x29, x30, [sp, #-0x20]!
001d8200: stp      x20, x19, [sp, #0x10]
```
