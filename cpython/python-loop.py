import sys

def test(v):
    y = 0
    for i in range(v):
        y = y + 1
    return y

res = test(ord(sys.argv[1]))

print(res, file=sys.stderr)
