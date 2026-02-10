def test(v):
    y = 0
    if v & 1:
        y = y - 1
    else:
        y = y + 2
    return y
