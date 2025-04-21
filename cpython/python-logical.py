import sys

def test(v):
    return (v & 0b1) and ((v >> 2) & 0b1)

res = test(ord(sys.argv[1]))

print(res, file=sys.stderr)
