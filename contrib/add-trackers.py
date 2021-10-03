#!/usr/bin/env python3
import sys


tracker = 'http://probably.loki:6680/announce'

f = sys.stdin.buffer


def next_byte():
    b = f.read(1)
    assert b is not None and len(b) == 1
    return b


def parse_int():
    s = b''
    x = next_byte()
    while x in b"0123456789":
        s += x
        x = next_byte()
    assert x == b'e' and len(s) > 0, "Invalid integer encoding"
    return int(s)


def parse_string(s):
    x = next_byte()
    while x in b"0123456789":
        s += x
        x = next_byte()
    assert x == b':', "Invalid string encoding"
    s = int(s)
    data = f.read(s)
    assert len(data) == s, "Truncated string data"
    return data


def parse_dict():
    d = {}
    last_key = None
    while True:
        t = next_byte()
        if t == b'e':
            return d
        assert t in b"0123456789", "Invalid dict: dict keys must be strings"
        raw_key = parse_string(t)
        if last_key is not None and raw_key <= last_key:
            raise Exception("Warning: found out-of-order dict keys ({} after {})".format(raw_key, last_key))
        last_key = raw_key
        t = next_byte()
        d[raw_key] = parse_thing(t)


def parse_list():
    l = []
    while True:
        t = next_byte()
        if t == b'e':
            return l
        l.append(parse_thing(t))


def parse_thing(t):
    if t == b'd':
        return parse_dict()
    if t == b'l':
        return parse_list()
    if t == b'i':
        return parse_int()
    if t in b"0123456789":
        return parse_string(t)
    assert False, "Parsing error: encountered invalid type '{}'".format(t)





def bencode(v, out):
    if isinstance(v, int):
        out.write('i{}e'.format(v).encode())
    if isinstance(v, bytes):
        out.write('{}:'.format(len(v)).encode())
        out.write(v)
    if isinstance(v, str):
        bencode(v.encode(), out)
    if isinstance(v, list):
        out.write(b'l')
        for e in v:
            bencode(e, out)
        out.write(b'e')
    if isinstance(v, dict):
        out.write(b'd')
        keys = list(v.keys())
        keys.sort()
        for key in keys:
            out.write('{}:'.format(len(key)).encode())
            out.write(key)
            bencode(v[key], out)
        out.write(b'e')

metadata = parse_thing(next_byte())

metadata[b'announce'] = tracker
metadata[b'announce-list'] = []

bencode(metadata, sys.stdout.buffer)
