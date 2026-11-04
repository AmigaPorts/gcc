#!/usr/bin/env python3
"""Run a freestanding AmigaOS-target ELF under QEMU's e500v2 CPU."""
import os, pathlib, socket, subprocess, sys, tempfile, time
elf = pathlib.Path(sys.argv[1]).resolve()
nm = subprocess.check_output([os.environ.get('PPC_NM', 'ppc-amigaos-nm'), str(elf)], text=True)
done = next(int(line.split()[0],16) for line in nm.splitlines() if line.endswith(' test_done'))
with tempfile.TemporaryDirectory(prefix='spe-qemu-') as tmp:
    sockname = str(pathlib.Path(tmp)/'gdb.sock')
    log = open(pathlib.Path(tmp)/'qemu.log', 'w+')
    proc = subprocess.Popen([os.environ.get('QEMU_PPC', 'qemu-system-ppc'),'-M','ppce500','-cpu','e500v2','-m','128M','-kernel',str(elf),'-nographic','-monitor','none','-serial','null','-S','-gdb',f'unix:{sockname},server=on,wait=off'], stdout=log, stderr=log)
    try:
        for _ in range(100):
            if pathlib.Path(sockname).exists(): break
            if proc.poll() is not None:
                log.seek(0); raise RuntimeError(log.read())
            time.sleep(.05)
        conn=socket.socket(socket.AF_UNIX);conn.settimeout(20);conn.connect(sockname)
        def cmd(data):
            payload=data.encode(); conn.sendall(b'$'+payload+b'#'+f'{sum(payload)%256:02x}'.encode())
            while conn.recv(1)!=b'$': pass
            reply=b''
            while True:
                c=conn.recv(1)
                if c==b'#':break
                reply+=c
            checksum=conn.recv(2)
            assert int(checksum,16)==sum(reply)%256
            conn.sendall(b'+')
            return reply.decode()
        print('stop:',cmd('?'))
        assert cmd(f'Z0,{done:x},4')=='OK'
        reply=cmd('c')
        print('run:',reply)
        assert reply.startswith('T05'), reply
        pc_reply=cmd('p40')
        print('pc:',pc_reply)
        assert pc_reply and int(pc_reply,16)==done, (pc_reply,hex(done))
        regs=bytes.fromhex(cmd('g'))
        print('r3:',int.from_bytes(regs[12:16],'big'),'r4:',hex(int.from_bytes(regs[16:20],'big')))
        if len(sys.argv)>2:
            assert int.from_bytes(regs[12:16],'big')==int(sys.argv[2],0)
    finally:
        proc.terminate()
        try:proc.wait(timeout=5)
        except subprocess.TimeoutExpired:proc.kill();proc.wait()
        log.close()
