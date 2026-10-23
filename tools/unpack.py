#!/usr/bin/env python3
"""The game, unpacked by !RunImage's own decruncher.

    unpack.py           game/!Lemmings2/!RunImage -> unpacked/runimage (the
                        image that runs, at &8000)

!RunImage is a crunched absolute. At &8000 it checks with OS_GetEnv that
the slot reaches &13D350, then branches to &95E64, which copies a
decruncher (&350 bytes from &95EDC) to &13D000 and runs it there: it
prints "Please wait....", unpacks the data (a backwards bit stream from
the Amiga original, as in James Pond and Chuck Rock, checksummed into r5)
to &8100 upwards, copies the result down to &8000 and branches there.
Here that all runs under SuperSpider's tools/arm2.py, from &8000 until it
comes back to &8000.
"""
import os
import sys

SS = os.environ.get('SUPERSPIDER', os.path.expanduser('~/stuff/superspider'))
sys.path.insert(0, os.path.join(SS, 'tools'))
import arm2  # noqa: E402

TOP = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
APP = os.path.join(TOP, 'game', '!Lemmings2')
OUT = os.path.join(TOP, 'unpacked')
BASE, HIMEM = 0x8000, 0x180000


def find(d, name):
    """a RISC OS file in a host directory: any case, any ,load-exec suffix"""
    for f in os.listdir(d):
        if f.split(',')[0].lower() == name.lower():
            return os.path.join(d, f)
    raise FileNotFoundError(os.path.join(d, name))


def swi(m, n, pc):
    n &= ~0x20000
    if n == 0x10:                       # OS_GetEnv
        m.r[0], m.r[1], m.r[2] = 0, HIMEM, 0
    elif n in (0, 0x100 + 12) or 0x100 <= n < 0x200:     # OS_WriteC, OS_WriteI
        pass
    elif n == 6:                        # OS_Byte 135: character at the cursor, screen mode
        m.r[1], m.r[2] = 0, 12
    else:
        raise arm2.Stop('unpack: SWI &%X at &%X' % (n, pc))


def unpack(image):
    m = arm2.ARM(memsize=0x400000)
    m.mem[BASE:BASE + len(image)] = image
    m.swi_handler = swi
    m.r[13] = HIMEM
    m.r[15] = BASE
    m.step()
    while m.r[15] != BASE:
        m.step()
        if m.steps > 400_000_000:
            raise RuntimeError('unpack: no end, pc=&%X' % m.r[15])
    if m.r[5]:
        sys.exit('unpack: !RunImage fails its checksum (a damaged dump?)')
    return bytes(m.mem[BASE:m.r[0]])   # where the copy down to &8000 stopped


def main():
    os.makedirs(OUT, exist_ok=True)
    data = unpack(open(find(APP, '!RunImage'), 'rb').read())
    open(os.path.join(OUT, 'runimage'), 'wb').write(data)
    print('runimage: %d bytes at &%X' % (len(data), BASE))


if __name__ == '__main__':
    main()
