0x001d82a0:  stp      x29, x30, [sp, #-0x30]!
0x001d82a4:  str      x21, [sp, #0x10]
0x001d82a8:  stp      x20, x19, [sp, #0x20]
0x001d82ac:  mov      x29, sp
0x001d82b0:  mov      x19, x2
0x001d82b4:  mov      x20, x0
0x001d82b8:  mov      x21, x1
0x001d82bc:  cbz      x0, #0x1d82e0
0x001d82c0:  cbz      x19, #0x1d82e0
0x001d82c4:  adrp     x8, #0x279000
0x001d82c8:  ldr      x8, [x8, #0x4f8]
0x001d82cc:  ldr      x8, [x8, #8]
0x001d82d0:  cbz      x8, #0x1d82e0
0x001d82d4:  ldr      x9, [x8, #0xa8]
0x001d82d8:  cmp      x9, x19
0x001d82dc:  b.eq     #0x1d8304
0x001d82e0:  adrp     x8, #0x279000
0x001d82e4:  mov      x0, x20
0x001d82e8:  mov      x1, x21
0x001d82ec:  ldr      x3, [x8, #0x600]
0x001d82f0:  mov      x2, x19
0x001d82f4:  ldp      x20, x19, [sp, #0x20]
0x001d82f8:  ldr      x21, [sp, #0x10]
0x001d82fc:  ldp      x29, x30, [sp], #0x30
0x001d8300:  br       x3
0x001d8304:  ldr      x9, [x20, #0x70]
0x001d8308:  cbz      x9, #0x1d82e0
0x001d830c:  ldrb     w9, [x9, #0x79]
0x001d8310:  ldrb     w8, [x8, #0x79]
0x001d8314:  cmp      w9, w8
0x001d8318:  b.eq     #0x1d82e0
0x001d831c:  adrp     x8, #0x279000
0x001d8320:  ldrb     w8, [x8, #0x608]
0x001d8324:  cbz      w8, #0x1d82e0
0x001d8328:  bl       #0x1d833c
0x001d832c:  adrp     x8, #0x279000
0x001d8330:  ldr      w1, [x8, #0x60c]
0x001d8334:  bl       #0x1eb09c
0x001d8338:  b        #0x1d82e0
