#include "cpu.h"

void cCpu::adc(u8 value) {
    u8 result = a + value + (u8)c_flag;
    z_flag = (result & 0xFFu) == 0;
    n_flag = false;
    h_flag = ((a & 0xFu) + (value & 0xFu) + (u8)c_flag) > 0xFu;
    c_flag = (a + value + (u8)c_flag) > 0xFFu;
    a = result;
}

void cCpu::add(u8 value) {
    z_flag = ((a + value) & 0xFFu) == 0;
    n_flag = false;
    h_flag = (a & 0xFu) + (value & 0xFu) > 0xFu;
    c_flag = (a + value) > 0xFFu;
    a += value;
}

void cCpu::addhl(u16 value) {
    u16 hl_value = hl();
    n_flag = false;
    h_flag = ((hl_value & 0xFFFu) + (value & 0xFFFu)) > 0xFFFu;
    // h_flag=((hl_value>>8)+(value>>8))>0xFF;
    c_flag = (hl_value + value) > 0xFFFFu;
    hl(hl_value + value);
}

void cCpu::addsp(s8 value) {
    c_flag = ((sp & 0xFFu) + (u8)value) > 0xFFu;
    h_flag = ((sp & 0xFu) + ((u8)value & 0xFu)) > 0xFu;
    sp += value;
    z_flag = false;
    n_flag = false;
}

void cCpu::z8and(u8 value) {
    a &= value;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = true;
    c_flag = false;
}

void cCpu::bit(u8 bit_number, u8 register_value) {
    bit_number >>= 3u;
    z_flag = (register_value & (1u << bit_number)) == 0;
    n_flag = false;
    h_flag = true;
}

void cCpu::call(bool condition, u16 address) {
    if (condition) {
        push(pc);
        pc = address;
        mCyclesSum += 12;
    }
}

void cCpu::cp(u8 val) {
    z_flag = (a == val);
    n_flag = true;
    h_flag = (a & 0xFu) < (val & 0xFu);
    c_flag = a < val;
}

void cCpu::daa() {
    int result = a;
    if (n_flag) {
        if (h_flag) result = (result - 6) & 0xFF;
        if (c_flag) result -= 0x60;
    } else {
        if (h_flag || (result & 0xF) > 9) result += 6;
        if (c_flag || result > 0x9F) result += 0x60;
    }

    if ((result & 0x100) == 0x100) c_flag = true;
    a = result & 0xFF;
    z_flag = (a == 0);
    h_flag = false;
}

void cCpu::dec(u8 &reg) {
    z_flag = (reg == 1);
    n_flag = true;
    h_flag = (reg & 0xFu) == 0;
    reg -= 1;
}

void cCpu::dec(u8 &high_register, u8 &low_register) {
    u16 result = (high_register << 8u) | low_register;
    result -= 1;
    high_register = result >> 8u;
    low_register = result & 0xFFu;
}

void cCpu::dechl() {
    u8 result = mMemory->readByte(hl());
    dec(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::inc(u8 &reg) {
    z_flag = (reg == 0xFF);
    n_flag = false;
    h_flag = (reg & 0xFu) == 0xF;
    reg += 1;
}

void cCpu::inc(u8 &high_register, u8 &low_register) {
    u16 result = (high_register << 8) | low_register;
    result += 1;
    high_register = result >> 8;
    low_register = result & 0xFF;
}

void cCpu::inchl() {
    u8 result = mMemory->readByte(hl());
    inc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::jp(bool condition, u16 address) {
    if (condition) {
        pc = address;
        mCyclesSum += 4;
    }
}

void cCpu::jr(bool condition, s8 value) {
    if (condition) {
        pc += value;
        mCyclesSum += 20;
    }
}

void cCpu::ldhl(s8 value) {
    c_flag = ((sp & 0xFFu) + (u8)value > 0xFFu);
    h_flag = ((sp & 0xFu) + ((u8)value & 0xFu) > 0xFu);
    z_flag = false;
    n_flag = false;
    hl(sp + value);
}

void cCpu::ldnnsp(u16 value) {
    mMemory->writeByte(value, sp & 0xFFu);
    mMemory->writeByte(value + 1, sp >> 8u);
}

void cCpu::z8or(u8 value) {
    a |= value;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}

void cCpu::pop(u8 &high_register, u8 &low_register) {
    low_register = mMemory->readByte(sp);
    sp++;
    high_register = mMemory->readByte(sp);
    sp++;
}

void cCpu::popaf() {
    u8 flags_value;
    pop(a, flags_value);
    flags(flags_value);
}

void cCpu::push(u16 regs) {
    sp--;
    mMemory->writeByte(sp, regs >> 8);
    sp--;
    mMemory->writeByte(sp, regs & 0xFF);
}

void cCpu::res(u8 bit_number, u8 &reg) {
    reg &= ~(1u << (bit_number >> 3u));
}

void cCpu::reshl(u8 bit_number) {
    u8 result = mMemory->readByte(hl());
    res(bit_number, result);
    mMemory->writeByte(hl(), result);
}

void cCpu::ret(bool condition) {
    u8 high_byte, low_byte;
    if (condition) {
        pop(high_byte, low_byte);
        pc = (high_byte << 8) | low_byte;
    }
}

void cCpu::rlc(u8 &reg) {
    c_flag = (reg >> 7) & 1;
    reg = (reg << 1) | c_flag;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rlca() {
    c_flag = (a >> 7) & 1;
    a = (a << 1) | c_flag;
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rlchl() {
    u8 result = mMemory->readByte(hl());
    rlc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rl(u8 &reg) {
    u8 carry_flag_bit = (u8)c_flag;
    c_flag = (reg >> 7u) & 1u;
    reg = (reg << 1u) | carry_flag_bit;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rla() {
    u8 result = a >> 7;
    a = (a << 1) | c_flag;
    z_flag = false;
    c_flag = result & 1;
    n_flag = false;
    h_flag = false;
}

void cCpu::rlhl() {
    u8 result = mMemory->readByte(hl());
    rl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rrc(u8 &reg) {
    c_flag = reg & 1u;
    reg = (reg >> 1u) | (c_flag << 7u);
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rrca() {
    c_flag = a & 1u;
    a = (a >> 1u) | (c_flag << 7u);
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rrchl() {
    u8 result = mMemory->readByte(hl());
    rrc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rr(u8 &reg) {
    bool new_carry_flag = reg & 1u;      // Keep the first bit
    reg = (reg >> 1u) | (c_flag << 7u);  // Shift n right and put carry at the top
    c_flag = new_carry_flag;             // Put the kept n bit into carry
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::rra() {
    bool result = a & 1u;            // Keep the first bit
    a = (a >> 1u) | (c_flag << 7u);  // Shift n right and put carry at the top
    c_flag = result;                 // Put the kept n bit into carry
    z_flag = false;
    n_flag = false;
    h_flag = false;
}

void cCpu::rrhl() {
    u8 result = mMemory->readByte(hl());
    rr(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rst(u8 address) {
    call(true, address);
}

void cCpu::sbc(u8 value) {
    u16 result = a - value - c_flag;
    n_flag = true;
    // h_flag = (result & 0xF) > (a & 0xF);
    c_flag = (result > a);
    h_flag = ((a ^ value ^ (result & 0xFFu)) & 0x10u) != 0;

    a = result & 0xFFu;
    z_flag = (a == 0);
}

void cCpu::set(u8 bit, u8 &reg) {
    reg |= (1u << (bit >> 3u));
}

void cCpu::sethl(u8 bit_number) {
    u8 result = mMemory->readByte(hl());
    set(bit_number, result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sla(u8 &reg) {
    c_flag = (reg >> 7u) & 1u;
    reg <<= 1u;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::slahl() {
    u8 result = mMemory->readByte(hl());
    sla(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sra(u8 &reg) {
    c_flag = reg & 1u;
    reg = (reg >> 1u) | (reg & 0x80u);
    z_flag = (reg == 0);
    h_flag = false;
    n_flag = false;
}

void cCpu::srahl() {
    u8 result = mMemory->readByte(hl());
    sra(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::srl(u8 &reg) {
    c_flag = reg & 1u;
    reg >>= 1u;
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
}

void cCpu::srlhl() {
    u8 result = mMemory->readByte(hl());
    srl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sub(u8 value) {
    z_flag = (a == value);
    n_flag = true;
    h_flag = (value & 0xFu) > (a & 0xFu);
    c_flag = (value > a);
    a -= value;
}

void cCpu::swap(u8 &reg) {
    reg = ((reg & 0xFu) << 4u) | (reg >> 4u);
    z_flag = (reg == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}

void cCpu::swaphl() {
    u8 result = mMemory->readByte(hl());
    swap(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::z8xor(u8 val) {
    a ^= val;
    z_flag = (a == 0);
    n_flag = false;
    h_flag = false;
    c_flag = false;
}
