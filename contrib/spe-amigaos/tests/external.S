    .text
    .global external
external:
    lis 0,0x3ff8
    cmpw 3,0
    bne 1f
    cmpwi 4,0
    bne 1f
    lis 0,0x4010
    cmpw 5,0
    bne 1f
    cmpwi 6,5
    bne 1f
    lis 3,0x4021
    ori 3,3,0x8000
    li 4,0
    blr
1:  li 3,0
    li 4,0
    blr
