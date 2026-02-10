def test(v):
    return (v & 0b1) and ((v >> 2) & 0b1)
