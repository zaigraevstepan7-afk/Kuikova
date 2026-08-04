0x001d81fc:  stp      x29, x30, [sp, #-0x20]!
0x001d8200:  stp      x20, x19, [sp, #0x10]
0x001d8204:  mov      x29, sp
0x001d8208:  mov      x19, x0
0x001d820c:  bl       #0x1d81bc
0x001d8210:  cbz      x19, #0x1d8278
0x001d8214:  adrp     x8, #0x279000
0x001d8218:  ldrb     w8, [x8, #0x5d1]
0x001d821c:  cbz      w8, #0x1d8278
0x001d8220:  ldr      x8, [x19, #0x2b0]
0x001d8224:  cbz      x8, #0x1d8278
0x001d8228:  ldr      x8, [x8, #0xb8]
0x001d822c:  cbz      x8, #0x1d8278
0x001d8230:  ldr      x8, [x8, #0x30]
0x001d8234:  cbz      x8, #0x1d8278
0x001d8238:  ldr      x0, [x8, #0x50]
0x001d823c:  cbz      x0, #0x1d8278
0x001d8240:  adrp     x8, #0x279000
0x001d8244:  mov      x1, xzr
0x001d8248:  ldr      x20, [x8, #0x4f0]
0x001d824c:  movz     w8, #0xaefc
0x001d8250:  movk     w8, #0x5ff, lsl #16
0x001d8254:  add      x8, x20, x8
0x001d8258:  blr      x8
0x001d825c:  cbz      x0, #0x1d8278
0x001d8260:  movz     w8, #0x9294
0x001d8264:  mov      w1, wzr
0x001d8268:  mov      x2, xzr
0x001d826c:  movk     w8, #0x600, lsl #16
0x001d8270:  add      x8, x20, x8
0x001d8274:  blr      x8
0x001d8278:  adrp     x8, #0x279000
0x001d827c:  ldr      x1, [x8, #0x5d8]
0x001d8280:  cbz      x1, #0x1d8294
0x001d8284:  mov      x0, x19
0x001d8288:  ldp      x20, x19, [sp, #0x10]
0x001d828c:  ldp      x29, x30, [sp], #0x20
0x001d8290:  br       x1
0x001d8294:  ldp      x20, x19, [sp, #0x10]
0x001d8298:  ldp      x29, x30, [sp], #0x20
0x001d829c:  ret      
