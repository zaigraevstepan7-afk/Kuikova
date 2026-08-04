0x001d84cc: stp      x29, x30, [sp, #-0x40]!
0x001d84d0: str      x23, [sp, #0x10]
0x001d84d4: stp      x22, x21, [sp, #0x20]
0x001d84d8: stp      x20, x19, [sp, #0x30]
0x001d84dc: mov      x29, sp
0x001d84e0: adrp     x0, #0x36000
0x001d84e4: add      x0, x0, #0x373  ; "libEGL.so"
0x001d84e8: adrp     x1, #0x35000
0x001d84ec: add      x1, x1, #0x75b  ; "eglSwapBuffers"
0x001d84f0: bl       #0x26c550  ; DobbySymbolResolver
0x001d84f4: cbz      x0, #0x1d850c
0x001d84f8: nop      
0x001d84fc: adr      x1, #0x1d76f0  ; 0x1d76f0
0x001d8500: nop      
0x001d8504: adr      x2, #0x279578  ; 0x279578
0x001d8508: bl       #0x26c560  ; DobbyHook
0x001d850c: adrp     x0, #0x30000
0x001d8510: add      x0, x0, #0x4a4  ; "libinput.so"
0x001d8514: adrp     x1, #0x2d000
0x001d8518: add      x1, x1, #0x3ff  ; "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEbl"
0x001d851c: bl       #0x26c550  ; DobbySymbolResolver
0x001d8520: cbz      x0, #0x1d8538
0x001d8524: nop      
0x001d8528: adr      x1, #0x1d760c  ; 0x1d760c
0x001d852c: nop      
0x001d8530: adr      x2, #0x279580  ; 0x279580
0x001d8534: bl       #0x26c560  ; DobbyHook
0x001d8538: adrp     x22, #0x279000
0x001d853c: ldr      x21, [x22, #0x4f0]
0x001d8540: cbz      x21, #0x1d85e0
0x001d8544: movz     w23, #0xc40c
0x001d8548: nop      
0x001d854c: adr      x19, #0x1d7a0c  ; 0x1d7a0c
0x001d8550: movk     w23, #0x8e7, lsl #16
0x001d8554: nop      
0x001d8558: adr      x20, #0x2795a8  ; 0x2795a8
0x001d855c: add      x0, x21, x23
0x001d8560: mov      x1, x19
0x001d8564: mov      x2, x20
0x001d8568: bl       #0x26c560  ; DobbyHook
0x001d856c: ldr      x2, [x20]
0x001d8570: add      x0, x21, x23
0x001d8574: mov      x1, x19
0x001d8578: bl       #0x1d917c  ; 0x1d917c
0x001d857c: ldr      x8, [x22, #0x4f0]
0x001d8580: movz     w9, #0x85c
0x001d8584: movk     w9, #0x8e0, lsl #16
0x001d8588: adds     x19, x8, x9
0x001d858c: b.eq     #0x1d85f4
0x001d8590: nop      
0x001d8594: adr      x20, #0x1d81fc  ; 0x1d81fc
0x001d8598: nop      
0x001d859c: adr      x21, #0x2795d8  ; 0x2795d8
0x001d85a0: mov      x0, x19
0x001d85a4: mov      x1, x20
0x001d85a8: mov      x2, x21
0x001d85ac: bl       #0x26c560  ; DobbyHook
0x001d85b0: mov      x0, x19
0x001d85b4: mov      x1, x20
0x001d85b8: mov      x2, x21
0x001d85bc: bl       #0x1d917c  ; 0x1d917c
0x001d85c0: ldr      x8, [x22, #0x4f0]
0x001d85c4: movz     w9, #0xe5e0
0x001d85c8: movk     w9, #0x79f, lsl #16
0x001d85cc: adds     x19, x8, x9
0x001d85d0: b.ne     #0x1d85fc
0x001d85d4: movz     w19, #0xe970
0x001d85d8: movk     w19, #0x147, lsl #16
0x001d85dc: b        #0x1d863c
0x001d85e0: ldp      x20, x19, [sp, #0x30]
0x001d85e4: ldr      x23, [sp, #0x10]
0x001d85e8: ldp      x22, x21, [sp, #0x20]
0x001d85ec: ldp      x29, x30, [sp], #0x40
0x001d85f0: ret      
0x001d85f4: movn     x19, #0x227b
0x001d85f8: movk     x19, #0xfebf, lsl #16
0x001d85fc: nop      
0x001d8600: adr      x20, #0x1d8404  ; 0x1d8404
0x001d8604: nop      
0x001d8608: adr      x21, #0x279520  ; 0x279520
0x001d860c: mov      x0, x19
0x001d8610: mov      x1, x20
0x001d8614: mov      x2, x21
0x001d8618: bl       #0x26c560  ; DobbyHook
0x001d861c: ldr      x2, [x21]
0x001d8620: mov      x0, x19
0x001d8624: mov      x1, x20
0x001d8628: bl       #0x1d917c  ; 0x1d917c
0x001d862c: ldr      x8, [x22, #0x4f0]
0x001d8630: add      x8, x23, x8
0x001d8634: adds     x19, x8, #0xb44
0x001d8638: b.eq     #0x1d868c
0x001d863c: nop      
0x001d8640: adr      x20, #0x1d7ec4  ; 0x1d7ec4
0x001d8644: nop      
0x001d8648: adr      x21, #0x2795f0  ; 0x2795f0
0x001d864c: mov      x0, x19
0x001d8650: mov      x1, x20
0x001d8654: mov      x2, x21
0x001d8658: bl       #0x26c560  ; DobbyHook
0x001d865c: ldr      x2, [x21]
0x001d8660: mov      x0, x19
0x001d8664: mov      x1, x20
0x001d8668: bl       #0x1d917c  ; 0x1d917c
0x001d866c: ldr      x8, [x22, #0x4f0]
0x001d8670: movz     w9, #0x63ec
0x001d8674: movk     w9, #0x8d6, lsl #16
0x001d8678: adds     x19, x8, x9
0x001d867c: b.ne     #0x1d8694
0x001d8680: movn     x19, #0xb13b
0x001d8684: movk     x19, #0xfffc, lsl #16
0x001d8688: b        #0x1d86d8
0x001d868c: movn     x19, #0x6b63
0x001d8690: movk     x19, #0xffee, lsl #16
0x001d8694: nop      
0x001d8698: adr      x20, #0x1d82a0  ; 0x1d82a0
0x001d869c: nop      
0x001d86a0: adr      x21, #0x279600  ; 0x279600
0x001d86a4: mov      x0, x19
0x001d86a8: mov      x1, x20
0x001d86ac: mov      x2, x21
0x001d86b0: bl       #0x26c560  ; DobbyHook
0x001d86b4: mov      x0, x19
0x001d86b8: mov      x1, x20
0x001d86bc: mov      x2, x21
0x001d86c0: bl       #0x1d917c  ; 0x1d917c
0x001d86c4: ldr      x8, [x22, #0x4f0]
0x001d86c8: movz     w9, #0xb2b0
0x001d86cc: movk     w9, #0x8d2, lsl #16
0x001d86d0: adds     x19, x8, x9
0x001d86d4: b.eq     #0x1d8708
0x001d86d8: nop      
0x001d86dc: adr      x20, #0x1d83cc  ; 0x1d83cc
0x001d86e0: nop      
0x001d86e4: adr      x21, #0x279610  ; 0x279610
0x001d86e8: mov      x0, x19
0x001d86ec: mov      x1, x20
0x001d86f0: mov      x2, x21
0x001d86f4: bl       #0x26c560  ; DobbyHook
0x001d86f8: mov      x0, x19
0x001d86fc: mov      x1, x20
0x001d8700: mov      x2, x21
0x001d8704: bl       #0x1d917c  ; 0x1d917c
0x001d8708: bl       #0x1d833c  ; 0x1d833c
0x001d870c: adrp     x2, #0x37000
0x001d8710: add      x2, x2, #0xfd2  ; 0x37fd2
0x001d8714: mov      w1, wzr
0x001d8718: movz     w3, #0xa294
0x001d871c: bl       #0x1eac8c  ; 0x1eac8c
0x001d8720: bl       #0x1d833c  ; 0x1d833c
0x001d8724: movz     w3, #0x892
0x001d8728: adrp     x2, #0x42000
0x001d872c: add      x2, x2, #0x266  ; 0x42266
0x001d8730: movz     w1, #0x1
0x001d8734: movk     w3, #0x1, lsl #16
0x001d8738: bl       #0x1eac8c  ; 0x1eac8c
0x001d873c: bl       #0x1d833c  ; 0x1d833c
0x001d8740: movz     w3, #0x5268
0x001d8744: adrp     x2, #0x52000
0x001d8748: add      x2, x2, #0xaf8  ; 0x52af8
