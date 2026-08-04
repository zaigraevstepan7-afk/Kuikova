# egl_install

- VA: `0x1d84cc`
- Size scanned: `0x200`
- Pass A instructions: 128
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 2 |

## Pass B — game RVAs (mov+movk)

- `w23` = `0x8e7c40c` @ `0x1d8550`
- `w9` = `0x8e0085c` @ `0x1d8584`
- `w9` = `0x79fe5e0` @ `0x1d85c8`
- `w19` = `0x147e970` @ `0x1d85d8`
- `w9` = `0x8d663ec` @ `0x1d8674`

## Pass B — strings (ADRP+ADD resolved only)

- `libEGL.so`
- `eglSwapBuffers`
- `libinput.so`
- `_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE`

## Pass B — calls (first 40)

- `0x1d84f0` → `0x26c550` (bl)
- `0x1d8508` → `0x26c560` (bl)
- `0x1d851c` → `0x26c550` (bl)
- `0x1d8534` → `0x26c560` (bl)
- `0x1d8568` → `0x26c560` (bl)
- `0x1d8578` → `0x1d917c` (bl)
- `0x1d85ac` → `0x26c560` (bl)
- `0x1d85bc` → `0x1d917c` (bl)
- `0x1d8618` → `0x26c560` (bl)
- `0x1d8628` → `0x1d917c` (bl)
- `0x1d8658` → `0x26c560` (bl)
- `0x1d8668` → `0x1d917c` (bl)
- `0x1d86b0` → `0x26c560` (bl)
- `0x1d86c0` → `0x1d917c` (bl)

## Pass A — full disasm

```
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
001d8504: adr      x2, #0x279578
001d8508: bl       #0x26c560
001d850c: adrp     x0, #0x30000
001d8510: add      x0, x0, #0x4a4
001d8514: adrp     x1, #0x2d000
001d8518: add      x1, x1, #0x3ff
001d851c: bl       #0x26c550
001d8520: cbz      x0, #0x1d8538
001d8524: nop      
001d8528: adr      x1, #0x1d760c
001d852c: nop      
001d8530: adr      x2, #0x279580
001d8534: bl       #0x26c560
001d8538: adrp     x22, #0x279000
001d853c: ldr      x21, [x22, #0x4f0]
001d8540: cbz      x21, #0x1d85e0
001d8544: mov      w23, #0xc40c
001d8548: nop      
001d854c: adr      x19, #0x1d7a0c
001d8550: movk     w23, #0x8e7, lsl #16
001d8554: nop      
001d8558: adr      x20, #0x2795a8
001d855c: add      x0, x21, x23
001d8560: mov      x1, x19
001d8564: mov      x2, x20
001d8568: bl       #0x26c560
001d856c: ldr      x2, [x20]
001d8570: add      x0, x21, x23
001d8574: mov      x1, x19
001d8578: bl       #0x1d917c
001d857c: ldr      x8, [x22, #0x4f0]
001d8580: mov      w9, #0x85c
001d8584: movk     w9, #0x8e0, lsl #16
001d8588: adds     x19, x8, x9
001d858c: b.eq     #0x1d85f4
001d8590: nop      
001d8594: adr      x20, #0x1d81fc
001d8598: nop      
001d859c: adr      x21, #0x2795d8
001d85a0: mov      x0, x19
001d85a4: mov      x1, x20
001d85a8: mov      x2, x21
001d85ac: bl       #0x26c560
001d85b0: mov      x0, x19
001d85b4: mov      x1, x20
001d85b8: mov      x2, x21
001d85bc: bl       #0x1d917c
001d85c0: ldr      x8, [x22, #0x4f0]
001d85c4: mov      w9, #0xe5e0
001d85c8: movk     w9, #0x79f, lsl #16
001d85cc: adds     x19, x8, x9
001d85d0: b.ne     #0x1d85fc
001d85d4: mov      w19, #0xe970
001d85d8: movk     w19, #0x147, lsl #16
001d85dc: b        #0x1d863c
001d85e0: ldp      x20, x19, [sp, #0x30]
001d85e4: ldr      x23, [sp, #0x10]
001d85e8: ldp      x22, x21, [sp, #0x20]
001d85ec: ldp      x29, x30, [sp], #0x40
001d85f0: ret      
001d85f4: mov      x19, #-0x227c
001d85f8: movk     x19, #0xfebf, lsl #16
001d85fc: nop      
001d8600: adr      x20, #0x1d8404
001d8604: nop      
001d8608: adr      x21, #0x279520
001d860c: mov      x0, x19
001d8610: mov      x1, x20
001d8614: mov      x2, x21
001d8618: bl       #0x26c560
001d861c: ldr      x2, [x21]
001d8620: mov      x0, x19
001d8624: mov      x1, x20
001d8628: bl       #0x1d917c
001d862c: ldr      x8, [x22, #0x4f0]
001d8630: add      x8, x23, x8
001d8634: adds     x19, x8, #0xb44
001d8638: b.eq     #0x1d868c
001d863c: nop      
001d8640: adr      x20, #0x1d7ec4
001d8644: nop      
001d8648: adr      x21, #0x2795f0
001d864c: mov      x0, x19
001d8650: mov      x1, x20
001d8654: mov      x2, x21
001d8658: bl       #0x26c560
001d865c: ldr      x2, [x21]
001d8660: mov      x0, x19
001d8664: mov      x1, x20
001d8668: bl       #0x1d917c
001d866c: ldr      x8, [x22, #0x4f0]
001d8670: mov      w9, #0x63ec
001d8674: movk     w9, #0x8d6, lsl #16
001d8678: adds     x19, x8, x9
001d867c: b.ne     #0x1d8694
001d8680: mov      x19, #-0xb13c
001d8684: movk     x19, #0xfffc, lsl #16
001d8688: b        #0x1d86d8
001d868c: mov      x19, #-0x6b64
001d8690: movk     x19, #0xffee, lsl #16
001d8694: nop      
001d8698: adr      x20, #0x1d82a0
001d869c: nop      
001d86a0: adr      x21, #0x279600
001d86a4: mov      x0, x19
001d86a8: mov      x1, x20
001d86ac: mov      x2, x21
001d86b0: bl       #0x26c560
001d86b4: mov      x0, x19
001d86b8: mov      x1, x20
001d86bc: mov      x2, x21
001d86c0: bl       #0x1d917c
001d86c4: ldr      x8, [x22, #0x4f0]
001d86c8: mov      w9, #0xb2b0
```
