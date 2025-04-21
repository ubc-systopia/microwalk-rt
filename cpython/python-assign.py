import sys

def test(v):
    y = 0
    y = 1 if v & 0b1 else y + 2
    return y

res = test(ord(sys.argv[1]))

print(res, file=sys.stderr)
