0x001d7ec4: sub      sp, sp, #0xa0
0x001d7ec8: stp      d9, d8, [sp, #0x30]
0x001d7ecc: stp      x29, x30, [sp, #0x40]
0x001d7ed0: stp      x28, x27, [sp, #0x50]
0x001d7ed4: stp      x26, x25, [sp, #0x60]
0x001d7ed8: stp      x24, x23, [sp, #0x70]
0x001d7edc: stp      x22, x21, [sp, #0x80]
0x001d7ee0: stp      x20, x19, [sp, #0x90]
0x001d7ee4: add      x29, sp, #0x40
0x001d7ee8: mrs      x26, tpidr_el0
0x001d7eec: mov      x19, x0
0x001d7ef0: ldr      x8, [x26, #0x28]  ; mem
0x001d7ef4: stur     x8, [x29, #-0x18]
0x001d7ef8: adrp     x8, #0x279000
0x001d7efc: ldr      x8, [x8, #0x5f0]  ; mem
0x001d7f00: cbz      x8, #0x1d7f0c
0x001d7f04: mov      x0, x19
0x001d7f08: blr      x8  ; indirect call
0x001d7f0c: adrp     x8, #0x279000
0x001d7f10: ldr      x8, [x8, #0x4f8]  ; mem
0x001d7f14: ldr      x8, [x8, #8]  ; mem
0x001d7f18: cmp      x8, x19
0x001d7f1c: b.eq     #0x1d7f54
0x001d7f20: ldr      x8, [x26, #0x28]  ; mem
0x001d7f24: ldur     x9, [x29, #-0x18]
0x001d7f28: cmp      x8, x9
0x001d7f2c: b.ne     #0x1d81b8
0x001d7f30: ldp      x20, x19, [sp, #0x90]
0x001d7f34: ldp      x22, x21, [sp, #0x80]
0x001d7f38: ldp      x24, x23, [sp, #0x70]
0x001d7f3c: ldp      x26, x25, [sp, #0x60]
0x001d7f40: ldp      x28, x27, [sp, #0x50]
0x001d7f44: ldp      x29, x30, [sp, #0x40]
0x001d7f48: ldp      d9, d8, [sp, #0x30]
0x001d7f4c: add      sp, sp, #0xa0
0x001d7f50: ret      
0x001d7f54: adrp     x8, #0x279000
0x001d7f58: mov      x1, x19
0x001d7f5c: ldr      x0, [x8, #0x500]  ; mem
0x001d7f60: bl       #0x1e0bcc  ; -> 0x1e0bcc
0x001d7f64: adrp     x8, #0x279000
0x001d7f68: ldrb     w8, [x8, #0x5f8]  ; mem
0x001d7f6c: cbz      w8, #0x1d7fbc
0x001d7f70: ldr      x0, [x19, #0x160]  ; mem
0x001d7f74: cbz      x0, #0x1d7fc8
0x001d7f78: bl       #0x1d88fc  ; -> 0x1d88fc
0x001d7f7c: cmp      w0, #1
0x001d7f80: b.lt     #0x1d7fc8
0x001d7f84: mov      x0, x19
0x001d7f88: mov      x1, x19
0x001d7f8c: bl       #0x1d8ee4  ; -> 0x1d8ee4
0x001d7f90: bl       #0x1d8f2c  ; -> 0x1d8f2c
0x001d7f94: cbz      x0, #0x1d7fc8
0x001d7f98: mov      x1, x0
0x001d7f9c: bl       #0x1d8f48  ; -> 0x1d8f48
0x001d7fa0: cbz      x0, #0x1d7fc8
0x001d7fa4: movi     d0, #0000000000000000
0x001d7fa8: movi     d1, #0000000000000000
0x001d7fac: mov      x1, x0
0x001d7fb0: fmov     s2, #-2.00000000
0x001d7fb4: bl       #0x1d8f74  ; -> 0x1d8f74
0x001d7fb8: b        #0x1d7fc8
0x001d7fbc: mov      x0, x19
0x001d7fc0: mov      x1, x19
0x001d7fc4: bl       #0x1d8f08  ; -> 0x1d8f08
0x001d7fc8: adrp     x20, #0x279000
0x001d7fcc: ldrb     w8, [x20, #0x5f9]  ; mem
0x001d7fd0: cbz      w8, #0x1d7fe4
0x001d7fd4: adrp     x8, #0x279000
0x001d7fd8: ldr      x0, [x8, #0x528]  ; mem
0x001d7fdc: bl       #0x1d9c7c  ; -> 0x1d9c7c
0x001d7fe0: strb     wzr, [x20, #0x5f9]  ; mem
0x001d7fe4: adrp     x8, #0x279000
0x001d7fe8: ldrb     w8, [x8, #0x5fa]  ; mem
0x001d7fec: cbz      w8, #0x1d7f20
0x001d7ff0: ldr      x21, [x19, #0x160]  ; mem
0x001d7ff4: cbz      x21, #0x1d7f20
0x001d7ff8: ldr      x8, [x21, #0x38]  ; mem
0x001d7ffc: cbz      x8, #0x1d7f20
0x001d8000: adrp     x1, #0x37000
0x001d8004: add      x1, x1, #0xf64
0x001d8008: mov      x0, x21
0x001d800c: bl       #0x1d8858  ; -> 0x1d8858
0x001d8010: mov      x22, x0
0x001d8014: adrp     x1, #0x37000
0x001d8018: add      x1, x1, #0xf94
0x001d801c: mov      x0, x21
0x001d8020: bl       #0x1d8858  ; -> 0x1d8858
0x001d8024: mov      x20, x0
0x001d8028: adrp     x1, #0x37000
0x001d802c: add      x1, x1, #0xfbc
0x001d8030: mov      x0, x21
0x001d8034: bl       #0x1d8858  ; -> 0x1d8858
0x001d8038: cbz      x22, #0x1d804c
0x001d803c: ldrb     w8, [x22, #0x10]  ; mem
0x001d8040: cmp      w8, #0
0x001d8044: cset     w21, ne
0x001d8048: b        #0x1d8050
0x001d804c: mov      w21, wzr
0x001d8050: movi     d9, #0000000000000000
0x001d8054: movi     d8, #0000000000000000
0x001d8058: cbz      x20, #0x1d8060
0x001d805c: ldr      d8, [x20, #0x10]  ; mem
0x001d8060: cbz      x0, #0x1d8068
0x001d8064: ldr      d9, [x0, #0x10]  ; mem
0x001d8068: adrp     x27, #0x279000
0x001d806c: movz     w24, #0xbe04
0x001d8070: mov      x0, xzr
0x001d8074: ldr      x8, [x27, #0x4f0]  ; mem
0x001d8078: movk     w24, #0x8c7, lsl #16
0x001d807c: add      x8, x8, x24
0x001d8080: add      x8, x8, #0x470
0x001d8084: blr      x8  ; indirect call
0x001d8088: fsub     d0, d0, d9
0x001d808c: fcmp     d0, d8
0x001d8090: csel     w8, wzr, w21, pl
0x001d8094: tbnz     w8, #0, #0x1d7f20
0x001d8098: ldr      x25, [x27, #0x4f0]  ; mem
0x001d809c: movz     w8, #0x58d8
0x001d80a0: movz     w9, #0xb5c0
0x001d80a4: movk     w8, #0xb02, lsl #16
0x001d80a8: movz     w28, #0xafc4
0x001d80ac: movk     w9, #0xac4, lsl #16
0x001d80b0: add      x8, x25, x8
0x001d80b4: movk     w28, #0x5ca, lsl #16
0x001d80b8: ldr      x0, [x25, x9]
0x001d80bc: ldr      x23, [x8]
0x001d80c0: ldr      x22, [x8, #0x190]  ; mem
0x001d80c4: ldr      x21, [x8, #0x98]  ; mem
0x001d80c8: add      x8, x25, x28
0x001d80cc: movz     w1, #0x5
0x001d80d0: blr      x8  ; indirect call
0x001d80d4: cbz      x0, #0x1d7f20
0x001d80d8: movz     w8, #0x64
0x001d80dc: movz     w9, #0x1
0x001d80e0: mov      x20, x0
0x001d80e4: stur     w8, [x29, #-0x1c]
0x001d80e8: add      x10, x25, x24
0x001d80ec: sub      x1, x29, #0x1c
0x001d80f0: str      w8, [sp, #0x20]  ; mem
0x001d80f4: add      x8, x28, x25
0x001d80f8: mov      x0, x23
0x001d80fc: strb     w9, [sp, #0x1c]  ; mem
