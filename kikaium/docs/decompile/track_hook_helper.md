# track_hook_helper

- VA: `0x1d917c`
- Size scanned: `0x80`
- Pass A instructions: 32
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 2 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

_none_

## Pass B — calls (first 40)

_none_

## Pass A — full disasm

```
001d917c: stp      x29, x30, [sp, #-0x40]!
001d9180: str      x23, [sp, #0x10]
001d9184: stp      x22, x21, [sp, #0x20]
001d9188: stp      x20, x19, [sp, #0x30]
001d918c: mov      x29, sp
001d9190: adrp     x8, #0x279000
001d9194: add      x8, x8, #0x6e0
001d9198: mov      x19, x2
001d919c: ldp      x9, x8, [x8]
001d91a0: mov      x20, x1
001d91a4: mov      x21, x0
001d91a8: cmp      x9, x8
001d91ac: b.hs     #0x1d91c0
001d91b0: stp      x21, x20, [x9]
001d91b4: add      x20, x9, #0x18
001d91b8: str      x19, [x9, #0x10]
001d91bc: b        #0x1d9290
001d91c0: adrp     x22, #0x279000
001d91c4: mov      x12, #-0x5555555555555556
001d91c8: ldr      x11, [x22, #0x6d8]
001d91cc: movk     x12, #0xaaab
001d91d0: sub      x9, x9, x11
001d91d4: asr      x9, x9, #3
001d91d8: mul      x23, x9, x12
001d91dc: mov      x9, #-0x5555555555555556
001d91e0: movk     x9, #0xaaa, lsl #48
001d91e4: add      x10, x23, #1
001d91e8: cmp      x10, x9
001d91ec: b.hi     #0x1d92ac
001d91f0: sub      x8, x8, x11
001d91f4: asr      x8, x8, #3
001d91f8: mul      x8, x8, x12
```
