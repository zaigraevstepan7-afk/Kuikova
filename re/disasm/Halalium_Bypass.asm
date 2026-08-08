0x001d90b8:  stp      x29, x30, [sp, #-0x40]!
0x001d90bc:  str      x23, [sp, #0x10]
0x001d90c0:  stp      x22, x21, [sp, #0x20]
0x001d90c4:  stp      x20, x19, [sp, #0x30]
0x001d90c8:  mov      x29, sp
0x001d90cc:  mov      x19, x0
0x001d90d0:  adrp     x1, #0x34000
0x001d90d4:  add      x1, x1, #0xd18
0x001d90d8:  adrp     x2, #0x2e000
0x001d90dc:  add      x2, x2, #0x8c9
0x001d90e0:  movz     w0, #0x4
0x001d90e4:  bl       #0x26c540
0x001d90e8:  nop      
0x001d90ec:  adr      x20, #0x2796d8
0x001d90f0:  ldp      x21, x22, [x20]
0x001d90f4:  cmp      x21, x22
0x001d90f8:  b.eq     #0x1d9108
0x001d90fc:  ldr      x0, [x21], #0x18
0x001d9100:  bl       #0x26c590
0x001d9104:  b        #0x1d90f4
0x001d9108:  adrp     x8, #0x279000
0x001d910c:  mov      x0, x19
0x001d9110:  ldr      x8, [x8, #0x6f0]
0x001d9114:  blr      x8
0x001d9118:  ldp      x22, x23, [x20]
0x001d911c:  mov      x19, x0
0x001d9120:  cmp      x22, x23
0x001d9124:  b.eq     #0x1d9164
0x001d9128:  adrp     x20, #0x34000
0x001d912c:  add      x20, x20, #0xd18
0x001d9130:  adrp     x21, #0x31000
0x001d9134:  add      x21, x21, #0x626
0x001d9138:  ldp      x0, x1, [x22]
0x001d913c:  add      x2, x22, #0x10
0x001d9140:  bl       #0x26c560
0x001d9144:  mov      w3, w0
0x001d9148:  movz     w0, #0x4
0x001d914c:  mov      x1, x20
0x001d9150:  mov      x2, x21
0x001d9154:  bl       #0x26c540
0x001d9158:  add      x22, x22, #0x18
0x001d915c:  cmp      x22, x23
0x001d9160:  b.ne     #0x1d9138
0x001d9164:  mov      x0, x19
0x001d9168:  ldp      x20, x19, [sp, #0x30]
0x001d916c:  ldr      x23, [sp, #0x10]
0x001d9170:  ldp      x22, x21, [sp, #0x20]
0x001d9174:  ldp      x29, x30, [sp], #0x40
0x001d9178:  ret      
