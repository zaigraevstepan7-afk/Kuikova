# libunity_base_resolve

- VA: `0x1d6afc`
- Size scanned: `0x1c0`
- Pass A instructions: 112
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x8` | unknown | 1 |
| `0x10` | unknown | 1 |
| `0x20` | unknown | 1 |
| `0x28` | players_list / camera holder | 5 |
| `0x38` | unknown | 1 |
| `0x48` | unknown | 2 |
| `0x58` | unknown | 2 |
| `0x60` | unknown | 1 |
| `0x170` | unknown | 1 |
| `0x1f0` | unknown | 2 |
| `0x238` | unknown | 1 |
| `0x258` | unknown | 1 |
| `0x260` | unknown | 1 |
| `0x268` | unknown | 1 |
| `0x318` | unknown | 1 |
| `0x370` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

- `libunity.so`

## Pass B — calls (first 40)

- `0x1d6b24` → `0x26c3c0` (bl)
- `0x1d6b34` → `0x26c3d0` (bl)
- `0x1d6b68` → `0x26c3e0` (bl)
- `0x1d6bb4` → `0x1d6d44` (bl)
- `0x1d6bc8` → `0x234470` (bl)
- `0x1d6bd4` → `0x1d6dd0` (bl)
- `0x1d6bdc` → `0x1d6f40` (bl)
- `0x1d6bec` → `0x26c3a0` (bl)
- `0x1d6c18` → `0x26c3f0` (bl)
- `0x1d6c38` → `0x26c3f0` (bl)
- `0x1d6c3c` → `0x1d92b8` (bl)
- `0x1d6c40` → `0x1d84cc` (bl)
- `0x1d6c54` → `0x26c330` (bl)
- `0x1d6c74` → `0x1d6f40` (bl)
- `0x1d6c7c` → `0x1d6d04` (bl)

## Pass A — full disasm

```
001d6afc: stp      x29, x30, [sp, #-0x30]!
001d6b00: stp      x28, x21, [sp, #0x10]
001d6b04: stp      x20, x19, [sp, #0x20]
001d6b08: mov      x29, sp
001d6b0c: sub      sp, sp, #0x3c0
001d6b10: mrs      x20, tpidr_el0
001d6b14: mov      x19, x0
001d6b18: ldr      x8, [x20, #0x28]
001d6b1c: stur     x8, [x29, #-8]
001d6b20: str      x0, [sp, #8]
001d6b24: bl       #0x26c3c0
001d6b28: ldr      x1, [x19]
001d6b2c: str      xzr, [x19]
001d6b30: ldr      w0, [x0]
001d6b34: bl       #0x26c3d0
001d6b38: movi     v0.2d, #0000000000000000
001d6b3c: add      x19, sp, #0x1f0
001d6b40: add      x21, sp, #0x1f0
001d6b44: add      x8, x19, #0x108
001d6b48: add      x0, x19, #0x70
001d6b4c: mov      w1, wzr
001d6b50: mov      w2, #0x92
001d6b54: str      xzr, [sp, #0x1f0]
001d6b58: str      wzr, [sp, #0x258]
001d6b5c: stp      q0, q0, [x8]
001d6b60: stur     q0, [x21, #0x48]
001d6b64: stur     q0, [x21, #0x58]
001d6b68: bl       #0x26c3e0
001d6b6c: movi     v0.2d, #0000000000000000
001d6b70: mov      w8, #0x3f800000
001d6b74: add      x9, x19, #0x160
001d6b78: adrp     x19, #0x2c000
001d6b7c: add      x19, x19, #0xde0
001d6b80: str      w8, [sp, #0x318]
001d6b84: str      wzr, [sp, #0x370]
001d6b88: stp      q0, q0, [x21, #0x130]
001d6b8c: stp      q0, q0, [x21, #0x150]
001d6b90: str      q0, [x21, #0x170]
001d6b94: mov      w21, #0x6500
001d6b98: movk     w21, #0x1dcd, lsl #16
001d6b9c: stur     q0, [x9, #0x28]
001d6ba0: stur     q0, [x9, #0x38]
001d6ba4: stur     q0, [x9, #0x48]
001d6ba8: stur     q0, [x9, #0x58]
001d6bac: add      x0, sp, #0x10
001d6bb0: mov      x1, x19
001d6bb4: bl       #0x1d6d44
001d6bb8: add      x8, sp, #0x28
001d6bbc: add      x0, sp, #0x10
001d6bc0: mov      w1, wzr
001d6bc4: mov      w2, wzr
001d6bc8: bl       #0x234470
001d6bcc: add      x0, sp, #0x1f0
001d6bd0: add      x1, sp, #0x28
001d6bd4: bl       #0x1d6dd0
001d6bd8: add      x0, sp, #0x28
001d6bdc: bl       #0x1d6f40
001d6be0: ldrb     w8, [sp, #0x10]
001d6be4: tbz      w8, #0, #0x1d6bf0
001d6be8: ldr      x0, [sp, #0x20]
001d6bec: bl       #0x26c3a0
001d6bf0: ldr      x8, [sp, #0x1f0]
001d6bf4: cbz      x8, #0x1d6c10
001d6bf8: ldr      x9, [sp, #0x268]
001d6bfc: cbz      x9, #0x1d6c10
001d6c00: ldr      x9, [sp, #0x238]
001d6c04: cbz      x9, #0x1d6c10
001d6c08: ldr      x9, [sp, #0x260]
001d6c0c: cbnz     x9, #0x1d6c20
001d6c10: str      x21, [sp, #0x28]
001d6c14: add      x0, sp, #0x28
001d6c18: bl       #0x26c3f0
001d6c1c: b        #0x1d6bac
001d6c20: mov      w10, #0x9400
001d6c24: adrp     x9, #0x279000
001d6c28: movk     w10, #0x7735, lsl #16
001d6c2c: str      x8, [x9, #0x4f0]
001d6c30: str      x10, [sp, #0x28]
001d6c34: add      x0, sp, #0x28
001d6c38: bl       #0x26c3f0
001d6c3c: bl       #0x1d92b8
001d6c40: bl       #0x1d84cc
001d6c44: adrp     x19, #0x279000
001d6c48: ldr      x8, [x19, #0x4f8]
001d6c4c: cbnz     x8, #0x1d6c70
001d6c50: mov      w0, #0x68
001d6c54: bl       #0x26c330
001d6c58: movi     v0.2d, #0000000000000000
001d6c5c: str      xzr, [x0, #0x60]
001d6c60: str      x0, [x19, #0x4f8]
001d6c64: stp      q0, q0, [x0]
001d6c68: stp      q0, q0, [x0, #0x20]
001d6c6c: stp      q0, q0, [x0, #0x40]
001d6c70: add      x0, sp, #0x1f0
001d6c74: bl       #0x1d6f40
001d6c78: add      x0, sp, #8
001d6c7c: bl       #0x1d6d04
001d6c80: ldr      x8, [x20, #0x28]
001d6c84: ldur     x9, [x29, #-8]
001d6c88: cmp      x8, x9
001d6c8c: b.ne     #0x1d6ce8
001d6c90: mov      x0, xzr
001d6c94: add      sp, sp, #0x3c0
001d6c98: ldp      x20, x19, [sp, #0x20]
001d6c9c: ldp      x28, x21, [sp, #0x10]
001d6ca0: ldp      x29, x30, [sp], #0x30
001d6ca4: ret      
001d6ca8: b        #0x1d6cbc
001d6cac: mov      x19, x0
001d6cb0: b        #0x1d6cc8
001d6cb4: b        #0x1d6cbc
001d6cb8: b        #0x1d6cbc
```
