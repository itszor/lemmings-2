#!/usr/bin/env python3
"""The hard disc installation, as !Lemmings2's !Help describes it.

    install.py          game/Disk1, game/disk2, game/disk3 -> game/!Lemmings2/

From floppy the game asks for each disc in turn (DiskTest, and the l2DT
file each disc holds). Installed, the three discs' directories sit inside
!Lemmings2, where <Tribe$Dir>.Disk1 etc. find them; disk2's extra sound
effects are then used on a 4MB machine. weave.py copies each disc's
directories to game/ by name, so this moves them into place.

TOSEC's disk 1 also carries the Extend virus (1989): a module that copies
itself into application directories under random names (CodeRM,
MonitorRM, OSextend) with a !Boot that runs it. Those files, recognised
by the module's title "Extend", and the !Boot are removed.
"""
import os
import shutil

GAME = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'game')
APP = os.path.join(GAME, '!Lemmings2')


def is_extend(path):
    with open(path, 'rb') as f:
        head = f.read(0x40)
    return len(head) == 0x40 and head[0x2c:0x33] == b'Extend\0'


def main():
    for f in os.listdir(APP):
        p = os.path.join(APP, f)
        boot = f.split(',')[0] == '!Boot' and b'RMEnsure Extend' in open(p, 'rb').read()
        if os.path.isfile(p) and (boot or is_extend(p)):
            os.remove(p)
    for d in ('Disk1', 'disk2', 'disk3'):
        src, dst = os.path.join(GAME, d), os.path.join(APP, d)
        if os.path.isdir(src):
            shutil.rmtree(dst, ignore_errors=True)
            shutil.move(src, dst)
        elif not os.path.isdir(dst):
            raise SystemExit('install: no %s' % src)


if __name__ == '__main__':
    main()
