# secondary_hook_cb

- VA: `0x1d81fc`
- Size scanned: `0x200`
- Pass A instructions: 128
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 6 |
| `0x30` | Photon.isLocal (ldrb) / nested | 1 |
| `0x50` | unknown | 1 |
| `0x70` | unknown | 1 |
| `0x79` | Player.team (byte) | 2 |
| `0xa8` | Player.hit / weapon params | 1 |
| `0xb8` | unknown | 1 |
| `0x2b0` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

- `w8` = `0x5ffaefc` @ `0x1d8250`
- `w8` = `0x6009294` @ `0x1d826c`

## Pass B — strings (ADRP+ADD resolved only)

_none_

## Pass B — calls (first 40)

- `0x1d820c` → `0x1d81bc` (bl)
- `0x1d8258` → `x8` (blr)
- `0x1d8274` → `x8` (blr)
- `0x1d8328` → `0x1d833c` (bl)
- `0x1d8334` → `0x1eb09c` (bl)
- `0x1d8374` → `0x26c300` (bl)
- `0x1d8384` → `0x1ea9f4` (bl)
- `0x1d83a0` → `0x26c2e0` (bl)
- `0x1d83ac` → `0x26c310` (bl)
- `0x1d83c0` → `0x26c480` (bl)
- `0x1d83c8` → `0x26841c` (bl)
- `0x1d83e4` → `x8` (blr)

## Pass A — full disasm

```
001d81fc: stp      x29, x30, [sp, #-0x20]!
001d8200: stp      x20, x19, [sp, #0x10]
001d8204: mov      x29, sp
001d8208: mov      x19, x0
001d820c: bl       #0x1d81bc
001d8210: cbz      x19, #0x1d8278
001d8214: adrp     x8, #0x279000
001d8218: ldrb     w8, [x8, #0x5d1]
001d821c: cbz      w8, #0x1d8278
001d8220: ldr      x8, [x19, #0x2b0]
001d8224: cbz      x8, #0x1d8278
001d8228: ldr      x8, [x8, #0xb8]
001d822c: cbz      x8, #0x1d8278
001d8230: ldr      x8, [x8, #0x30]
001d8234: cbz      x8, #0x1d8278
001d8238: ldr      x0, [x8, #0x50]
001d823c: cbz      x0, #0x1d8278
001d8240: adrp     x8, #0x279000
001d8244: mov      x1, xzr
001d8248: ldr      x20, [x8, #0x4f0]
001d824c: mov      w8, #0xaefc
001d8250: movk     w8, #0x5ff, lsl #16
001d8254: add      x8, x20, x8
001d8258: blr      x8
001d825c: cbz      x0, #0x1d8278
001d8260: mov      w8, #0x9294
001d8264: mov      w1, wzr
001d8268: mov      x2, xzr
001d826c: movk     w8, #0x600, lsl #16
001d8270: add      x8, x20, x8
001d8274: blr      x8
001d8278: adrp     x8, #0x279000
001d827c: ldr      x1, [x8, #0x5d8]
001d8280: cbz      x1, #0x1d8294
001d8284: mov      x0, x19
001d8288: ldp      x20, x19, [sp, #0x10]
001d828c: ldp      x29, x30, [sp], #0x20
001d8290: br       x1
001d8294: ldp      x20, x19, [sp, #0x10]
001d8298: ldp      x29, x30, [sp], #0x20
001d829c: ret      
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
001d83d0: str      x19, [sp, #0x10]
001d83d4: mov      x29, sp
001d83d8: adrp     x8, #0x279000
001d83dc: mov      x19, x1
001d83e0: ldr      x8, [x8, #0x610]
001d83e4: blr      x8
001d83e8: adrp     x8, #0x279000
001d83ec: ldrb     w8, [x8, #0x618]
001d83f0: cbz      w8, #0x1d83f8
001d83f4: str      xzr, [x19]
001d83f8: ldr      x19, [sp, #0x10]
```
