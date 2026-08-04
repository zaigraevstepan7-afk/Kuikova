0x001d83cc:  stp      x29, x30, [sp, #-0x20]!
0x001d83d0:  str      x19, [sp, #0x10]
0x001d83d4:  mov      x29, sp
0x001d83d8:  adrp     x8, #0x279000
0x001d83dc:  mov      x19, x1
0x001d83e0:  ldr      x8, [x8, #0x610]
0x001d83e4:  blr      x8
0x001d83e8:  adrp     x8, #0x279000
0x001d83ec:  ldrb     w8, [x8, #0x618]
0x001d83f0:  cbz      w8, #0x1d83f8
0x001d83f4:  str      xzr, [x19]
0x001d83f8:  ldr      x19, [sp, #0x10]
0x001d83fc:  ldp      x29, x30, [sp], #0x20
0x001d8400:  ret      
0x001d8404:  adrp     x8, #0x279000
0x001d8408:  ldrb     w8, [x8, #0x5a1]
