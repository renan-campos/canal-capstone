#! /usr/bin/python

import sysv_ipc

# TODO add bluetooth logic here.

q = sysv_ipc.MessageQueue(0x7dd)

q.send(chr(10), type=1)
