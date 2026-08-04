# extraA_cb

- VA: `0x1d82a0`
- Size scanned: `0x130`
- Pass A instructions: 76
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 4 |
| `0x70` | unknown | 1 |
| `0x79` | Player.team (byte) | 2 |
| `0xa8` | Player.hit / weapon params | 1 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

_none_

## Pass B — calls (first 40)

- `0x1d8328` → `0x1d833c` (bl)
- `0x1d8334` → `0x1eb09c` (bl)
- `0x1d8374` → `0x26c300` (bl)
- `0x1d8384` → `0x1ea9f4` (bl)
- `0x1d83a0` → `0x26c2e0` (bl)
- `0x1d83ac` → `0x26c310` (bl)
- `0x1d83c0` → `0x26c480` (bl)
- `0x1d83c8` → `0x26841c` (bl)

## Pass A — full disasm

```
001d82a0: stp      x29, x30, [sp, #-0x30]!
001d82a4: str      x21, [sp, #0x10]
001d82a8: stp      x20, x19, [sp, #0x20]
001d82ac: mov      x29, sp
001d82b0: mov      x19, x2
001d82b4: mov      x20, x0
001d82b8: mov      x21, x1
001d82bc: cbz      x0, #0x1d82e0
001d82c0: cbz      x19, #0x1d82e0
001d82c4: adrp     x8, #0x279000
001d82c8: ldr      x8, [x8, #0x4f8]
001d82cc: ldr      x8, [x8, #8]
001d82d0: cbz      x8, #0x1d82e0
001d82d4: ldr      x9, [x8, #0xa8]
001d82d8: cmp      x9, x19
001d82dc: b.eq     #0x1d8304
001d82e0: adrp     x8, #0x279000
001d82e4: mov      x0, x20
001d82e8: mov      x1, x21
001d82ec: ldr      x3, [x8, #0x600]
001d82f0: mov      x2, x19
001d82f4: ldp      x20, x19, [sp, #0x20]
001d82f8: ldr      x21, [sp, #0x10]
001d82fc: ldp      x29, x30, [sp], #0x30
001d8300: br       x3
001d8304: ldr      x9, [x20, #0x70]
001d8308: cbz      x9, #0x1d82e0
001d830c: ldrb     w9, [x9, #0x79]
001d8310: ldrb     w8, [x8, #0x79]
001d8314: cmp      w9, w8
001d8318: b.eq     #0x1d82e0
001d831c: adrp     x8, #0x279000
001d8320: ldrb     w8, [x8, #0x608]
001d8324: cbz      w8, #0x1d82e0
001d8328: bl       #0x1d833c
001d832c: adrp     x8, #0x279000
001d8330: ldr      w1, [x8, #0x60c]
001d8334: bl       #0x1eb09c
001d8338: b        #0x1d82e0
001d833c: stp      x29, x30, [sp, #-0x20]!
001d8340: str      x19, [sp, #0x10]
001d8344: mov      x29, sp
001d8348: nop      
001d834c: adr      x8, #0x2796d0
001d8350: ldarb    w8, [x8]
001d8354: tbz      w8, #0, #0x1d836c
001d8358: nop      
001d835c: adr      x0, #0x279678
001d8360: ldr      x19, [sp, #0x10]
001d8364: ldp      x29, x30, [sp], #0x20
001d8368: ret      
001d836c: nop      
001d8370: adr      x0, #0x2796d0
001d8374: bl       #0x26c300
001d8378: cbz      w0, #0x1d8358
001d837c: nop      
001d8380: adr      x0, #0x279678
001d8384: bl       #0x1ea9f4
001d8388: adrp     x0, #0x277000
001d838c: nop      
001d8390: adr      x1, #0x279678
001d8394: ldr      x0, [x0, #0x488]
001d8398: nop      
001d839c: adr      x2, #0x26e5f0
001d83a0: bl       #0x26c2e0
001d83a4: nop      
001d83a8: adr      x0, #0x2796d0
001d83ac: bl       #0x26c310
001d83b0: b        #0x1d8358
001d83b4: mov      x19, x0
001d83b8: nop      
001d83bc: adr      x0, #0x2796d0
001d83c0: bl       #0x26c480
001d83c4: mov      x0, x19
001d83c8: bl       #0x26841c
001d83cc: stp      x29, x30, [sp, #-0x20]!
```
