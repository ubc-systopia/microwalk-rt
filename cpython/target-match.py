def test(v):
    y = 0
    match v & 0b11:
        case 0b00:
            y = y
        case 0b01:
            y = y + 1
        case 0b10:
            y = y - 2
        case 0b11:
            y = y + 1 - 2
    return y
