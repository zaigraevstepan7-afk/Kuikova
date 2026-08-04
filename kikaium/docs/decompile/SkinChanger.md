# SkinChanger

- VA: `0x1d9e00`
- Size scanned: `0x1b4`
- Pass A instructions: 109
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x8` | unknown | 1 |
| `0x10` | unknown | 2 |
| `0x14` | unknown | 1 |
| `0x18` | unknown | 2 |
| `0x28` | players_list / camera holder | 2 |
| `0x30` | Photon.isLocal (ldrb) / nested | 2 |
| `0x88` | Player.weaponry | 2 |
| `0xa0` | Weaponry.weapon OR arms | 1 |
| `0xa8` | Player.hit / weapon params | 1 |
| `0xe0` | unknown | 1 |
| `0x158` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

- `w9` = `0x8e8fe50` @ `0x1d9ef8`
- `w9` = `0x8e852d4` @ `0x1d9f28`
- `w9` = `0x8e7f7f4` @ `0x1d9f4c`

## Pass B — strings (ADRP+ADD resolved only)

- `SkinChanger`
- `Skin Changer: Swapped to weapon %d (skin %d)`

## Pass B — calls (first 40)

- `0x1d9e7c` → `0x1d9fb8` (bl)
- `0x1d9eac` → `0x1da06c` (bl)
- `0x1d9ecc` → `0x1da06c` (bl)
- `0x1d9f14` → `x8` (blr)
- `0x1d9f3c` → `x8` (blr)
- `0x1d9f60` → `x8` (blr)
- `0x1d9f80` → `0x26c540` (bl)

## Pass A — full disasm

```
001d9e00: sub      sp, sp, #0x80
001d9e04: stp      x29, x30, [sp, #0x20]
001d9e08: str      x27, [sp, #0x30]
001d9e0c: stp      x26, x25, [sp, #0x40]
001d9e10: stp      x24, x23, [sp, #0x50]
001d9e14: stp      x22, x21, [sp, #0x60]
001d9e18: stp      x20, x19, [sp, #0x70]
001d9e1c: add      x29, sp, #0x20
001d9e20: mrs      x24, tpidr_el0
001d9e24: ldr      x8, [x24, #0x28]
001d9e28: stur     x8, [x29, #-8]
001d9e2c: cbz      x0, #0x1d9f84
001d9e30: adrp     x8, #0x279000
001d9e34: ldrb     w8, [x8, #0x712]
001d9e38: cbz      w8, #0x1d9f84
001d9e3c: ldr      x19, [x0, #0x88]
001d9e40: mov      x20, x0
001d9e44: cbz      x19, #0x1d9f84
001d9e48: ldr      x25, [x19, #0xa0]
001d9e4c: cbz      x25, #0x1d9f84
001d9e50: ldr      x8, [x25, #0xa8]
001d9e54: cbz      x8, #0x1d9f84
001d9e58: ldrb     w26, [x8, #0x18]
001d9e5c: nop      
001d9e60: adr      x0, #0x279620
001d9e64: add      x1, sp, #8
001d9e68: add      x27, sp, #8
001d9e6c: sub      w8, w26, #0x46
001d9e70: cmp      w8, #0x14
001d9e74: csinv    w8, w26, wzr, hs
001d9e78: str      w8, [sp, #8]
001d9e7c: bl       #0x1d9fb8
001d9e80: cbz      x0, #0x1d9f84
001d9e84: nop      
001d9e88: adr      x22, #0x279620
001d9e8c: adrp     x23, #0x77000
001d9e90: add      x23, x23, #0xd60
001d9e94: add      x1, sp, #8
001d9e98: add      x3, sp, #0x10
001d9e9c: add      x4, sp, #0xf
001d9ea0: mov      x0, x22
001d9ea4: mov      x2, x23
001d9ea8: str      x27, [sp, #0x10]
001d9eac: bl       #0x1da06c
001d9eb0: ldr      w21, [x0, #0x14]
001d9eb4: add      x1, sp, #8
001d9eb8: add      x3, sp, #0x10
001d9ebc: add      x4, sp, #0xf
001d9ec0: mov      x0, x22
001d9ec4: mov      x2, x23
001d9ec8: str      x27, [sp, #0x10]
001d9ecc: bl       #0x1da06c
001d9ed0: ldr      w22, [x0, #0x18]
001d9ed4: cmp      w21, w26
001d9ed8: b.ne     #0x1d9ee8
001d9edc: ldr      w8, [x25, #0xe0]
001d9ee0: cmp      w8, w22
001d9ee4: b.eq     #0x1d9f84
001d9ee8: adrp     x23, #0x279000
001d9eec: mov      w9, #0xfe50
001d9ef0: ldr      w2, [x20, #0x158]
001d9ef4: ldr      x8, [x23, #0x4f0]
001d9ef8: movk     w9, #0x8e8, lsl #16
001d9efc: mov      x0, x19
001d9f00: mov      w1, w21
001d9f04: mov      w3, w22
001d9f08: mov      w4, wzr
001d9f0c: add      x8, x8, x9
001d9f10: mov      x5, xzr
001d9f14: blr      x8
001d9f18: cbz      x0, #0x1d9f84
001d9f1c: ldr      x8, [x23, #0x4f0]
001d9f20: mov      w9, #0x52d4
001d9f24: ldrb     w1, [x19, #0x88]
001d9f28: movk     w9, #0x8e8, lsl #16
001d9f2c: mov      x20, x0
001d9f30: mov      x0, x19
001d9f34: add      x8, x8, x9
001d9f38: mov      x2, xzr
001d9f3c: blr      x8
001d9f40: ldr      x8, [x23, #0x4f0]
001d9f44: mov      w9, #0xf7f4
001d9f48: mov      x0, x19
001d9f4c: movk     w9, #0x8e7, lsl #16
001d9f50: mov      x1, x20
001d9f54: mov      w2, wzr
001d9f58: add      x8, x8, x9
001d9f5c: mov      x3, xzr
001d9f60: blr      x8
001d9f64: adrp     x1, #0x35000
001d9f68: add      x1, x1, #0x221
001d9f6c: adrp     x2, #0x33000
001d9f70: add      x2, x2, #0x13e
001d9f74: mov      w0, #4
001d9f78: mov      w3, w21
001d9f7c: mov      w4, w22
001d9f80: bl       #0x26c540
001d9f84: ldr      x8, [x24, #0x28]
001d9f88: ldur     x9, [x29, #-8]
001d9f8c: cmp      x8, x9
001d9f90: b.ne     #0x1d9fb4
001d9f94: ldp      x20, x19, [sp, #0x70]
001d9f98: ldr      x27, [sp, #0x30]
001d9f9c: ldp      x22, x21, [sp, #0x60]
001d9fa0: ldp      x24, x23, [sp, #0x50]
001d9fa4: ldp      x26, x25, [sp, #0x40]
001d9fa8: ldp      x29, x30, [sp, #0x20]
001d9fac: add      sp, sp, #0x80
001d9fb0: ret      
```
