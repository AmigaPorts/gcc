    .text
    .global _start, test_done
_start:
    lis 1,0x0100
    mfmsr 0
    oris 0,0,0x0200
    mtmsr 0
    isync
    bl test_main
test_done:
    b test_done
