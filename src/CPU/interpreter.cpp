#include "cpu.h"

void cCpu::executeOpCode(int a_opCode) {
    switch (a_opCode) {
        case 0x06: b = readNextByte(); break;
        case 0x0E: c = readNextByte(); break;
        case 0x16: d = readNextByte(); break;
        case 0x1E: e = readNextByte(); break;
        case 0x26: h = readNextByte(); break;
        case 0x2E: l = readNextByte(); break;

        case 0x7F:  // a = a;
            break;
        case 0x78: a = b; break;
        case 0x79: a = c; break;
        case 0x7A: a = d; break;
        case 0x7B: a = e; break;
        case 0x7C: a = h; break;
        case 0x7D: a = l; break;
        case 0x7E: a = mMemory->readByte(hl()); break;

        case 0x40:  // b = b;
            break;
        case 0x41: b = c; break;
        case 0x42: b = d; break;
        case 0x43: b = e; break;
        case 0x44: b = h; break;
        case 0x45: b = l; break;
        case 0x46: b = mMemory->readByte(hl()); break;

        case 0x48: c = b; break;
        case 0x49:  // c = c;
            break;
        case 0x4A: c = d; break;
        case 0x4B: c = e; break;
        case 0x4C: c = h; break;
        case 0x4D: c = l; break;
        case 0x4E: c = mMemory->readByte(hl()); break;

        case 0x50: d = b; break;
        case 0x51: d = c; break;
        case 0x52:  // d = d;
            break;
        case 0x53: d = e; break;
        case 0x54: d = h; break;
        case 0x55: d = l; break;
        case 0x56: d = mMemory->readByte(hl()); break;

        case 0x58: e = b; break;
        case 0x59: e = c; break;
        case 0x5A: e = d; break;
        case 0x5B:  // e = e;
            break;
        case 0x5C: e = h; break;
        case 0x5D: e = l; break;
        case 0x5E: e = mMemory->readByte(hl()); break;

        case 0x60: h = b; break;
        case 0x61: h = c; break;
        case 0x62: h = d; break;
        case 0x63: h = e; break;
        case 0x64:  // h = h;
            break;
        case 0x65: h = l; break;
        case 0x66: h = mMemory->readByte(hl()); break;

        case 0x68: l = b; break;
        case 0x69: l = c; break;
        case 0x6A: l = d; break;
        case 0x6B: l = e; break;
        case 0x6C: l = h; break;
        case 0x6D:  // l = l;
            break;
        case 0x6E: l = mMemory->readByte(hl()); break;

        case 0x70: mMemory->writeByte(hl(), b); break;
        case 0x71: mMemory->writeByte(hl(), c); break;
        case 0x72: mMemory->writeByte(hl(), d); break;
        case 0x73: mMemory->writeByte(hl(), e); break;
        case 0x74: mMemory->writeByte(hl(), h); break;
        case 0x75: mMemory->writeByte(hl(), l); break;
        case 0x36: mMemory->writeByte(hl(), readNextByte()); break;

        case 0x0A: a = mMemory->readByte(bc()); break;
        case 0x1A: a = mMemory->readByte(de()); break;
        case 0xFA: a = mMemory->readByte(readNextWord()); break;
        case 0x3E: a = readNextByte(); break;

        case 0x47: b = a; break;
        case 0x4F: c = a; break;
        case 0x57: d = a; break;
        case 0x5F: e = a; break;
        case 0x67: h = a; break;
        case 0x6F: l = a; break;
        case 0x02: mMemory->writeByte(bc(), a); break;
        case 0x12: mMemory->writeByte(de(), a); break;
        case 0x77: mMemory->writeByte(hl(), a); break;
        case 0xEA: mMemory->writeByte(readNextWord(), a); break;

        case 0xF2: a = mMemory->readByte(0xFF00 | c); break;
        case 0xE2: mMemory->writeByte(0xFF00 | c, a); break;

        case 0x3A:
            a = mMemory->readByte(hl());
            hl(hl() - 1);
            break;
        case 0x32:
            mMemory->writeByte(hl(), a);
            hl(hl() - 1);
            break;
        case 0x2A:
            a = mMemory->readByte(hl());
            hl(hl() + 1);
            break;
        case 0x22:
            mMemory->writeByte(hl(), a);
            hl(hl() + 1);
            break;

        case 0xE0: mMemory->writeByte(0xFF00 | readNextByte(), a); break;
        case 0xF0: a = mMemory->readByte(0xFF00 | readNextByte()); break;

        case 0x01: bc(readNextWord()); break;
        case 0x11: de(readNextWord()); break;
        case 0x21: hl(readNextWord()); break;
        case 0x31: sp = readNextWord(); break;

        case 0xF9: sp = hl(); break;
        case 0xF8: ldhl(readNextByte()); break;
        case 0x08: ldnnsp(readNextWord()); break;

        case 0xF5: push(af()); break;
        case 0xC5: push(bc()); break;
        case 0xD5: push(de()); break;
        case 0xE5: push(hl()); break;

        case 0xF1: popaf(); break;
        case 0xC1: pop(b, c); break;
        case 0xD1: pop(d, e); break;
        case 0xE1: pop(h, l); break;

        case 0x87: add(a); break;
        case 0x80: add(b); break;
        case 0x81: add(c); break;
        case 0x82: add(d); break;
        case 0x83: add(e); break;
        case 0x84: add(h); break;
        case 0x85: add(l); break;
        case 0x86: add(mMemory->readByte(hl())); break;
        case 0xC6: add(readNextByte()); break;

        case 0x8F: adc(a); break;
        case 0x88: adc(b); break;
        case 0x89: adc(c); break;
        case 0x8A: adc(d); break;
        case 0x8B: adc(e); break;
        case 0x8C: adc(h); break;
        case 0x8D: adc(l); break;
        case 0x8E: adc(mMemory->readByte(hl())); break;
        case 0xCE: adc(readNextByte()); break;

        case 0x97: sub(a); break;
        case 0x90: sub(b); break;
        case 0x91: sub(c); break;
        case 0x92: sub(d); break;
        case 0x93: sub(e); break;
        case 0x94: sub(h); break;
        case 0x95: sub(l); break;
        case 0x96: sub(mMemory->readByte(hl())); break;
        case 0xD6: sub(readNextByte()); break;

        case 0x9F: sbc(a); break;
        case 0x98: sbc(b); break;
        case 0x99: sbc(c); break;
        case 0x9A: sbc(d); break;
        case 0x9B: sbc(e); break;
        case 0x9C: sbc(h); break;
        case 0x9D: sbc(l); break;
        case 0x9E: sbc(mMemory->readByte(hl())); break;
        case 0xDE: sbc(readNextByte()); break;

        case 0xA7: z8and(a); break;
        case 0xA0: z8and(b); break;
        case 0xA1: z8and(c); break;
        case 0xA2: z8and(d); break;
        case 0xA3: z8and(e); break;
        case 0xA4: z8and(h); break;
        case 0xA5: z8and(l); break;
        case 0xA6: z8and(mMemory->readByte(hl())); break;
        case 0xE6: z8and(readNextByte()); break;

        case 0xB7: z8or(a); break;
        case 0xB0: z8or(b); break;
        case 0xB1: z8or(c); break;
        case 0xB2: z8or(d); break;
        case 0xB3: z8or(e); break;
        case 0xB4: z8or(h); break;
        case 0xB5: z8or(l); break;
        case 0xB6: z8or(mMemory->readByte(hl())); break;
        case 0xF6: z8or(readNextByte()); break;

        case 0xAF: z8xor(a); break;
        case 0xA8: z8xor(b); break;
        case 0xA9: z8xor(c); break;
        case 0xAA: z8xor(d); break;
        case 0xAB: z8xor(e); break;
        case 0xAC: z8xor(h); break;
        case 0xAD: z8xor(l); break;
        case 0xAE: z8xor(mMemory->readByte(hl())); break;
        case 0xEE: z8xor(readNextByte()); break;

        case 0xBF: cp(a); break;
        case 0xB8: cp(b); break;
        case 0xB9: cp(c); break;
        case 0xBA: cp(d); break;
        case 0xBB: cp(e); break;
        case 0xBC: cp(h); break;
        case 0xBD: cp(l); break;
        case 0xBE: cp(mMemory->readByte(hl())); break;
        case 0xFE: cp(readNextByte()); break;

        case 0x3C: inc(a); break;
        case 0x04: inc(b); break;
        case 0x0C: inc(c); break;
        case 0x14: inc(d); break;
        case 0x1C: inc(e); break;
        case 0x24: inc(h); break;
        case 0x2C: inc(l); break;
        case 0x34: inchl(); break;

        case 0x3D: dec(a); break;
        case 0x05: dec(b); break;
        case 0x0D: dec(c); break;
        case 0x15: dec(d); break;
        case 0x1D: dec(e); break;
        case 0x25: dec(h); break;
        case 0x2D: dec(l); break;
        case 0x35: dechl(); break;

        case 0x09: addhl(bc()); break;
        case 0x19: addhl(de()); break;
        case 0x29: addhl(hl()); break;
        case 0x39: addhl(sp); break;
        case 0xE8: addsp(readNextByte()); break;

        case 0x03: inc(b, c); break;
        case 0x13: inc(d, e); break;
        case 0x23: inc(h, l); break;
        case 0x33: sp++; break;

        case 0x0B: dec(b, c); break;
        case 0x1B: dec(d, e); break;
        case 0x2B: dec(h, l); break;
        case 0x3B: sp--; break;

        case 0x27: daa(); break;
        case 0x2F:
            a = ~a;
            n_flag = true;
            h_flag = true;
            break;
        case 0x3F:
            c_flag = (c_flag == false);
            n_flag = false;
            h_flag = false;
            break;
        case 0x37:
            c_flag = true;
            n_flag = false;
            h_flag = false;
            break;

        case 0x00: break;

        case 0x76:  // HALT
            if (mMemory->getEnabledInterrupts() == 0) pc--;
            break;
        case 0x10: mMemory->changeSpeed(); break;
        case 0xF3: intStatus = 2; break;
        case 0xFB: intStatus = 1; break;

        case 0x07: rlca(); break;
        case 0x17: rla(); break;
        case 0x1F: rra(); break;
        case 0x0F: rrca(); break;

        case 0xC3: pc = readNextWord(); break;
        case 0xC2: jp(z_flag == false, readNextWord()); break;
        case 0xCA: jp(z_flag == true, readNextWord()); break;
        case 0xD2: jp(c_flag == false, readNextWord()); break;
        case 0xDA: jp(c_flag == true, readNextWord()); break;
        case 0xE9: pc = hl(); break;

        case 0x18: jr(true, readNextByte()); break;
        case 0x20: jr(z_flag == false, readNextByte()); break;
        case 0x28: jr(z_flag == true, readNextByte()); break;
        case 0x30: jr(c_flag == false, readNextByte()); break;
        case 0x38: jr(c_flag == true, readNextByte()); break;

        case 0xCD: call(true, readNextWord()); break;
        case 0xC4: call(z_flag == false, readNextWord()); break;
        case 0xCC: call(z_flag == true, readNextWord()); break;
        case 0xD4: call(c_flag == false, readNextWord()); break;
        case 0xDC: call(c_flag == true, readNextWord()); break;

        case 0xC7: rst(0x00); break;
        case 0xCF: rst(0x08); break;
        case 0xD7: rst(0x10); break;
        case 0xDF: rst(0x18); break;
        case 0xE7: rst(0x20); break;
        case 0xEF: rst(0x28); break;
        case 0xF7: rst(0x30); break;
        case 0xFF: rst(0x38); break;

        case 0xC9: ret(true); break;
        case 0xC0: ret(z_flag == false); break;
        case 0xC8: ret(z_flag == true); break;
        case 0xD0: ret(c_flag == false); break;
        case 0xD8: ret(c_flag == true); break;

        case 0xD9:
            ret(true);
            interruptsEnabled = true;
            break;

        default:  // TODO: Raise exception
            WARNING(true, "Unkown opcode");
    }
}

void cCpu::executeCBOpCode(u8 a_cbOpCode) {
    u8 innerBits = 0;
    switch (a_cbOpCode) {
        case 0x37: swap(a); break;
        case 0x30: swap(b); break;
        case 0x31: swap(c); break;
        case 0x32: swap(d); break;
        case 0x33: swap(e); break;
        case 0x34: swap(h); break;
        case 0x35: swap(l); break;
        case 0x36: swaphl(); break;

        case 0x07: rlc(a); break;
        case 0x00: rlc(b); break;
        case 0x01: rlc(c); break;
        case 0x02: rlc(d); break;
        case 0x03: rlc(e); break;
        case 0x04: rlc(h); break;
        case 0x05: rlc(l); break;
        case 0x06: rlchl(); break;

        case 0x17: rl(a); break;
        case 0x10: rl(b); break;
        case 0x11: rl(c); break;
        case 0x12: rl(d); break;
        case 0x13: rl(e); break;
        case 0x14: rl(h); break;
        case 0x15: rl(l); break;
        case 0x16: rlhl(); break;

        case 0x0F: rrc(a); break;
        case 0x08: rrc(b); break;
        case 0x09: rrc(c); break;
        case 0x0A: rrc(d); break;
        case 0x0B: rrc(e); break;
        case 0x0C: rrc(h); break;
        case 0x0D: rrc(l); break;
        case 0x0E: rrchl(); break;

        case 0x1F: rr(a); break;
        case 0x18: rr(b); break;
        case 0x19: rr(c); break;
        case 0x1A: rr(d); break;
        case 0x1B: rr(e); break;
        case 0x1C: rr(h); break;
        case 0x1D: rr(l); break;
        case 0x1E: rrhl(); break;

        case 0x27: sla(a); break;
        case 0x20: sla(b); break;
        case 0x21: sla(c); break;
        case 0x22: sla(d); break;
        case 0x23: sla(e); break;
        case 0x24: sla(h); break;
        case 0x25: sla(l); break;
        case 0x26: slahl(); break;

        case 0x2F: sra(a); break;
        case 0x28: sra(b); break;
        case 0x29: sra(c); break;
        case 0x2A: sra(d); break;
        case 0x2B: sra(e); break;
        case 0x2C: sra(h); break;
        case 0x2D: sra(l); break;
        case 0x2E: srahl(); break;

        case 0x3F: srl(a); break;
        case 0x38: srl(b); break;
        case 0x39: srl(c); break;
        case 0x3A: srl(d); break;
        case 0x3B: srl(e); break;
        case 0x3C: srl(h); break;
        case 0x3D: srl(l); break;
        case 0x3E: srlhl(); break;

        default:
            innerBits = a_cbOpCode & 0x38;
            switch (a_cbOpCode & 0xC7) {  // Discard the bit number
                case 0x47: bit(innerBits, a); break;
                case 0x40: bit(innerBits, b); break;
                case 0x41: bit(innerBits, c); break;
                case 0x42: bit(innerBits, d); break;
                case 0x43: bit(innerBits, e); break;
                case 0x44: bit(innerBits, h); break;
                case 0x45: bit(innerBits, l); break;
                case 0x46: bit(innerBits, mMemory->readByte(hl())); break;

                case 0xC7: set(innerBits, a); break;
                case 0xC0: set(innerBits, b); break;
                case 0xC1: set(innerBits, c); break;
                case 0xC2: set(innerBits, d); break;
                case 0xC3: set(innerBits, e); break;
                case 0xC4: set(innerBits, h); break;
                case 0xC5: set(innerBits, l); break;
                case 0xC6: sethl(innerBits); break;

                case 0x87: res(innerBits, a); break;
                case 0x80: res(innerBits, b); break;
                case 0x81: res(innerBits, c); break;
                case 0x82: res(innerBits, d); break;
                case 0x83: res(innerBits, e); break;
                case 0x84: res(innerBits, h); break;
                case 0x85: res(innerBits, l); break;
                case 0x86: reshl(innerBits); break;
                default:  // TODO: Raise exception.
                    std::cout << "Opcode " << std::hex << a_cbOpCode << "no ejecutado" << std::endl;
            }
    }
}
