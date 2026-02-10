def test(v):
    y = 0
    y = 1 if v & 0b1 else y + 2
    return y
