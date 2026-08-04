0x001d8404:  adrp     x8, #0x279000
0x001d8408:  ldrb     w8, [x8, #0x5a1]
0x001d840c:  cbz      w8, #0x1d84c0
0x001d8410:  movz     w8, #0x447a, lsl #16
0x001d8414:  fmov     s1, w8
0x001d8418:  fcmp     s0, s1
0x001d841c:  b.ne     #0x1d84c0
0x001d8420:  movz     w8, #0x6010
0x001d8424:  movk     w8, #0x6000, lsl #16
0x001d8428:  cmp      w2, w8
0x001d842c:  b.ne     #0x1d84c0
0x001d8430:  cbz      x0, #0x1d84c0
0x001d8434:  adrp     x8, #0x279000
0x001d8438:  ldrb     w8, [x8, #0x5cc]
0x001d843c:  cbz      w8, #0x1d84c0
0x001d8440:  adrp     x8, #0x279000
0x001d8444:  ldr      w8, [x8, #0x5e8]
0x001d8448:  cbnz     w8, #0x1d84c0
0x001d844c:  nop      
0x001d8450:  adr      x8, #0x279490
0x001d8454:  ldr      d2, [x0]
0x001d8458:  ldr      d1, [x8]
0x001d845c:  ldr      s4, [x0, #8]
0x001d8460:  fsub     v1.2s, v1.2s, v2.2s
0x001d8464:  fmul     v2.2s, v1.2s, v1.2s
0x001d8468:  mov      s3, v2.s[1]
0x001d846c:  ldr      s2, [x8, #8]
0x001d8470:  adrp     x8, #0x37000
0x001d8474:  fsub     s2, s2, s4
0x001d8478:  ldr      s4, [x8, #0xf14]
0x001d847c:  fmadd    s3, s1, s1, s3
0x001d8480:  fmadd    s3, s2, s2, s3
0x001d8484:  fsqrt    s3, s3
0x001d8488:  fcmp     s3, s4
0x001d848c:  b.le     #0x1d84c0
0x001d8490:  adrp     x8, #0x37000
0x001d8494:  ldr      s4, [x8, #0xf3c]
0x001d8498:  fcmp     s3, s4
0x001d849c:  b.le     #0x1d84b0
0x001d84a0:  dup      v4.2s, v3.s[0]
0x001d84a4:  fdiv     s2, s2, s3
0x001d84a8:  fdiv     v1.2s, v1.2s, v4.2s
0x001d84ac:  b        #0x1d84b8
0x001d84b0:  movi     d1, #0000000000000000
0x001d84b4:  movi     d2, #0000000000000000
0x001d84b8:  stur     d1, [x0, #0xc]
0x001d84bc:  str      s2, [x0, #0x14]
0x001d84c0:  adrp     x8, #0x279000
0x001d84c4:  ldr      x4, [x8, #0x520]
0x001d84c8:  br       x4
0x001d84cc:  stp      x29, x30, [sp, #-0x40]!
0x001d84d0:  str      x23, [sp, #0x10]
0x001d84d4:  stp      x22, x21, [sp, #0x20]
0x001d84d8:  stp      x20, x19, [sp, #0x30]
0x001d84dc:  mov      x29, sp
0x001d84e0:  adrp     x0, #0x36000
0x001d84e4:  add      x0, x0, #0x373
0x001d84e8:  adrp     x1, #0x35000
0x001d84ec:  add      x1, x1, #0x75b
0x001d84f0:  bl       #0x26c550
0x001d84f4:  cbz      x0, #0x1d850c
0x001d84f8:  nop      
0x001d84fc:  adr      x1, #0x1d76f0
0x001d8500:  nop      
0x001d8504:  adr      x2, #0x279578
0x001d8508:  bl       #0x26c560
0x001d850c:  adrp     x0, #0x30000
0x001d8510:  add      x0, x0, #0x4a4
0x001d8514:  adrp     x1, #0x2d000
0x001d8518:  add      x1, x1, #0x3ff
0x001d851c:  bl       #0x26c550
0x001d8520:  cbz      x0, #0x1d8538
0x001d8524:  nop      
0x001d8528:  adr      x1, #0x1d760c
0x001d852c:  nop      
0x001d8530:  adr      x2, #0x279580
0x001d8534:  bl       #0x26c560
0x001d8538:  adrp     x22, #0x279000
0x001d853c:  ldr      x21, [x22, #0x4f0]
0x001d8540:  cbz      x21, #0x1d85e0
0x001d8544:  movz     w23, #0xc40c
0x001d8548:  nop      
0x001d854c:  adr      x19, #0x1d7a0c
0x001d8550:  movk     w23, #0x8e7, lsl #16
0x001d8554:  nop      
0x001d8558:  adr      x20, #0x2795a8
0x001d855c:  add      x0, x21, x23
0x001d8560:  mov      x1, x19
0x001d8564:  mov      x2, x20
0x001d8568:  bl       #0x26c560
0x001d856c:  ldr      x2, [x20]
0x001d8570:  add      x0, x21, x23
0x001d8574:  mov      x1, x19
0x001d8578:  bl       #0x1d917c
0x001d857c:  ldr      x8, [x22, #0x4f0]
0x001d8580:  movz     w9, #0x85c
0x001d8584:  movk     w9, #0x8e0, lsl #16
0x001d8588:  adds     x19, x8, x9
0x001d858c:  b.eq     #0x1d85f4
0x001d8590:  nop      
0x001d8594:  adr      x20, #0x1d81fc
0x001d8598:  nop      
0x001d859c:  adr      x21, #0x2795d8
0x001d85a0:  mov      x0, x19
0x001d85a4:  mov      x1, x20
0x001d85a8:  mov      x2, x21
0x001d85ac:  bl       #0x26c560
0x001d85b0:  mov      x0, x19
0x001d85b4:  mov      x1, x20
0x001d85b8:  mov      x2, x21
0x001d85bc:  bl       #0x1d917c
0x001d85c0:  ldr      x8, [x22, #0x4f0]
0x001d85c4:  movz     w9, #0xe5e0
0x001d85c8:  movk     w9, #0x79f, lsl #16
0x001d85cc:  adds     x19, x8, x9
0x001d85d0:  b.ne     #0x1d85fc
0x001d85d4:  movz     w19, #0xe970
0x001d85d8:  movk     w19, #0x147, lsl #16
0x001d85dc:  b        #0x1d863c
0x001d85e0:  ldp      x20, x19, [sp, #0x30]
0x001d85e4:  ldr      x23, [sp, #0x10]
0x001d85e8:  ldp      x22, x21, [sp, #0x20]
0x001d85ec:  ldp      x29, x30, [sp], #0x40
0x001d85f0:  ret      
