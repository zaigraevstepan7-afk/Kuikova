# Bypass_getrr

- VA: `0x1d90b8`
- Size scanned: `0x100`
- Pass A instructions: 64
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 3 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

- `Halalium_Bypass`
- `got call from getrr.`
- `bypas hok result %d`

## Pass B — calls (first 40)

- `0x1d90e4` → `0x26c540` (bl)
- `0x1d9100` → `0x26c590` (bl)
- `0x1d9114` → `x8` (blr)
- `0x1d9140` → `0x26c560` (bl)
- `0x1d9154` → `0x26c540` (bl)

## Pass A — full disasm

```
001d90b8: stp      x29, x30, [sp, #-0x40]!
001d90bc: str      x23, [sp, #0x10]
001d90c0: stp      x22, x21, [sp, #0x20]
001d90c4: stp      x20, x19, [sp, #0x30]
001d90c8: mov      x29, sp
001d90cc: mov      x19, x0
001d90d0: adrp     x1, #0x34000
001d90d4: add      x1, x1, #0xd18
001d90d8: adrp     x2, #0x2e000
001d90dc: add      x2, x2, #0x8c9
001d90e0: mov      w0, #4
001d90e4: bl       #0x26c540
001d90e8: nop      
001d90ec: adr      x20, #0x2796d8
001d90f0: ldp      x21, x22, [x20]
001d90f4: cmp      x21, x22
001d90f8: b.eq     #0x1d9108
001d90fc: ldr      x0, [x21], #0x18
001d9100: bl       #0x26c590
001d9104: b        #0x1d90f4
001d9108: adrp     x8, #0x279000
001d910c: mov      x0, x19
001d9110: ldr      x8, [x8, #0x6f0]
001d9114: blr      x8
001d9118: ldp      x22, x23, [x20]
001d911c: mov      x19, x0
001d9120: cmp      x22, x23
001d9124: b.eq     #0x1d9164
001d9128: adrp     x20, #0x34000
001d912c: add      x20, x20, #0xd18
001d9130: adrp     x21, #0x31000
001d9134: add      x21, x21, #0x626
001d9138: ldp      x0, x1, [x22]
001d913c: add      x2, x22, #0x10
001d9140: bl       #0x26c560
001d9144: mov      w3, w0
001d9148: mov      w0, #4
001d914c: mov      x1, x20
001d9150: mov      x2, x21
001d9154: bl       #0x26c540
001d9158: add      x22, x22, #0x18
001d915c: cmp      x22, x23
001d9160: b.ne     #0x1d9138
001d9164: mov      x0, x19
001d9168: ldp      x20, x19, [sp, #0x30]
001d916c: ldr      x23, [sp, #0x10]
001d9170: ldp      x22, x21, [sp, #0x20]
001d9174: ldp      x29, x30, [sp], #0x40
001d9178: ret      
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
```
