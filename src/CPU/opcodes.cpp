#include "cpu.h"

void cCpu::adc(const u8 value) noexcept {
    const u16 full_result = a + value + f.flags.c;
    const u8 result = static_cast<u8>(full_result);
    f.flags.z = result == 0;
    f.flags.n = false;
    f.flags.h = ((a & 0xFu) + (value & 0xFu) + f.flags.c) > 0xFu;
    f.flags.c = full_result > 0xFFu;
    a = result;
}

void cCpu::add(u8 value) {
    f.flags.z = ((a + value) & 0xFFu) == 0;
    f.flags.n = false;
    f.flags.h = (a & 0xFu) + (value & 0xFu) > 0xFu;
    f.flags.c = (a + value) > 0xFFu;
    a += value;
}

void cCpu::addhl(u16 value) {
    u16 hl_value = hl();
    f.flags.n = false;
    f.flags.h = ((hl_value & 0xFFFu) + (value & 0xFFFu)) > 0xFFFu;
    // h_flag=((hl_value>>8)+(value>>8))>0xFF;
    f.flags.c = (hl_value + value) > 0xFFFFu;
    hl(hl_value + value);
}

void cCpu::addsp(s8 value) {
    f.flags.c = ((sp & 0xFFu) + static_cast<u8>(value)) > 0xFFu;
    f.flags.h = ((sp & 0xFu) + (static_cast<u8>(value) & 0xFu)) > 0xFu;
    sp += value;
    f.flags.z = false;
    f.flags.n = false;
}

void cCpu::z8and(u8 value) {
    a &= value;
    f.flags.z = (a == 0);
    f.flags.n = false;
    f.flags.h = true;
    f.flags.c = false;
}

void cCpu::bit(u8 bit_number, u8 register_value) {
    bit_number >>= 3u;
    f.flags.z = (register_value & (1u << bit_number)) == 0;
    f.flags.n = false;
    f.flags.h = true;
}

void cCpu::call(const bool condition, const u16 address) noexcept {
    if (condition) [[unlikely]] {
        push(pc);
        pc = address;
        mCyclesSum += 12;
    }
}

void cCpu::cp(u8 val) {
    f.flags.z = (a == val);
    f.flags.n = true;
    f.flags.h = (a & 0xFu) < (val & 0xFu);
    f.flags.c = a < val;
}

void cCpu::daa() {
    int result = a;
    if (f.flags.n) {
        if (f.flags.h) result = (result - 6) & 0xFF;
        if (f.flags.c) result -= 0x60;
    } else {
        if (f.flags.h || (result & 0xF) > 9) result += 6;
        if (f.flags.c || result > 0x9F) result += 0x60;
    }

    if ((result & 0x100) == 0x100) {
        f.flags.c = true;
    }
    a = result & 0xFF;
    f.flags.z = (a == 0);
    f.flags.h = false;
}

void cCpu::dec(u8 &reg) {
    f.flags.z = (reg == 1);
    f.flags.n = true;
    f.flags.h = (reg & 0xFu) == 0;
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
    f.flags.z = (reg == 0xFF);
    f.flags.n = false;
    f.flags.h = (reg & 0xFu) == 0xF;
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
    f.flags.c = ((sp & 0xFFu) + static_cast<u8>(value) > 0xFFu);
    f.flags.h = ((sp & 0xFu) + (static_cast<u8>(value) & 0xFu) > 0xFu);
    f.flags.z = false;
    f.flags.n = false;
    hl(sp + value);
}

void cCpu::ldnnsp(u16 value) {
    mMemory->writeByte(value, sp & 0xFFu);
    mMemory->writeByte(value + 1, sp >> 8u);
}

void cCpu::z8or(u8 value) {
    a |= value;
    f.flags.z = (a == 0);
    f.flags.n = false;
    f.flags.h = false;
    f.flags.c = false;
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
    f.byte = flags_value & 0xF0u;
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
    f.flags.c = (reg >> 7) & 1;
    reg = (reg << 1) | f.flags.c;
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rlca() {
    f.flags.c = (a >> 7) & 1;
    a = (a << 1) | f.flags.c;
    f.flags.z = false;
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rlchl() {
    u8 result = mMemory->readByte(hl());
    rlc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rl(u8 &reg) {
    u8 carry_flag_bit = f.flags.c;
    f.flags.c = (reg >> 7u) & 1u;
    reg = (reg << 1u) | carry_flag_bit;
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rla() {
    u8 result = a >> 7;
    a = (a << 1) | f.flags.c;
    f.flags.z = false;
    f.flags.c = result & 1;
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rlhl() {
    u8 result = mMemory->readByte(hl());
    rl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rrc(u8 &reg) {
    f.flags.c = reg & 1u;
    reg = (reg >> 1u) | (f.flags.c << 7u);
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rrca() {
    f.flags.c = a & 1u;
    a = (a >> 1u) | (f.flags.c << 7u);
    f.flags.z = false;
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rrchl() {
    u8 result = mMemory->readByte(hl());
    rrc(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::rr(u8 &reg) {
    bool new_carry_flag = reg & 1u;      // Keep the first bit
    reg = (reg >> 1u) | (f.flags.c << 7u);  // Shift n right and put carry at the top
    f.flags.c = new_carry_flag;             // Put the kept n bit into carry
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::rra() {
    bool result = a & 1u;            // Keep the first bit
    a = (a >> 1u) | (f.flags.c << 7u);  // Shift n right and put carry at the top
    f.flags.c = result;                 // Put the kept n bit into carry
    f.flags.z = false;
    f.flags.n = false;
    f.flags.h = false;
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
    u16 result = a - value - f.flags.c;
    f.flags.n = true;
    // h_flag = (result & 0xF) > (a & 0xF);
    f.flags.c = (result > a);
    f.flags.h = ((a ^ value ^ (result & 0xFFu)) & 0x10u) != 0;

    a = result & 0xFFu;
    f.flags.z = (a == 0);
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
    f.flags.c = (reg >> 7u) & 1u;
    reg <<= 1u;
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::slahl() {
    u8 result = mMemory->readByte(hl());
    sla(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sra(u8 &reg) {
    f.flags.c = reg & 1u;
    reg = (reg >> 1u) | (reg & 0x80u);
    f.flags.z = (reg == 0);
    f.flags.h = false;
    f.flags.n = false;
}

void cCpu::srahl() {
    u8 result = mMemory->readByte(hl());
    sra(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::srl(u8 &reg) {
    f.flags.c = reg & 1u;
    reg >>= 1u;
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
}

void cCpu::srlhl() {
    u8 result = mMemory->readByte(hl());
    srl(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::sub(u8 value) {
    f.flags.z = (a == value);
    f.flags.n = true;
    f.flags.h = (value & 0xFu) > (a & 0xFu);
    f.flags.c = (value > a);
    a -= value;
}

void cCpu::swap(u8 &reg) {
    reg = ((reg & 0xFu) << 4u) | (reg >> 4u);
    f.flags.z = (reg == 0);
    f.flags.n = false;
    f.flags.h = false;
    f.flags.c = false;
}

void cCpu::swaphl() {
    u8 result = mMemory->readByte(hl());
    swap(result);
    mMemory->writeByte(hl(), result);
}

void cCpu::z8xor(u8 val) {
    a ^= val;
    f.flags.z = (a == 0);
    f.flags.n = false;
    f.flags.h = false;
    f.flags.c = false;
}
