#ifndef BIOGB_CPU
#define BIOGB_CPU

#include <expected>

#include "../memory/memory_map.h"
#include "../input.h"
#include "../macros.h"
#include "../sound/sound.h"

union FRegister {
    struct {
        u8: 4; // Lower 4 bits are unused
        u8 c: 1; // Carry flag
        u8 h: 1; // Half-carry flag
        u8 n: 1; // Subtract flag
        u8 z: 1; // Zero flag
    } flags;

    u8 byte{};

    constexpr FRegister() noexcept = default;

    constexpr explicit FRegister(const u8 value) noexcept : byte(value) {
    };
};

class cCpu {
   public:
    cCpu() noexcept;
    ~cCpu() = default;
    [[nodiscard]] std::expected<void, std::string> init_cpu(std::string_view file_name);

    [[nodiscard]] std::span<const u32> get_video_buffer() const {return mMemory->mDisplay->get_video_buffer();}
    [[nodiscard]] cSound *get_sound_system() const {return mMemory->mSound.get();}
    void update_input(const GBKey input_key) const;
    void reset_input() {mMemory->mInput->reset_input();}

    void saveState(int number);
    void loadState(int number);
    void runFrame();
    void runScanLine();
    int fetchOpCode();

   private:
    std::unique_ptr<MemoryMap> mMemory;
    std::array<int, 0x100> mOpcodeCyclesTable;
    std::array<int, 0x100> mCBOpcodeCyclesTable;
    u8 a{}, b{}, c{}, d{}, e{}, h{}, l{};
    FRegister f{};
    u16 pc{}, sp{};
    bool interruptsEnabled{};
    u32 intStatus{};
    int mCyclesSum{};
    s32 rtcCount{};
    // Help routines
    u16 af();
    void af(u16);
    u16 bc();
    void bc(u16);
    u16 de();
    void de(u16);
    u16 hl();
    void hl(u16);
    u8 readNextByte();
    u16 readNextWord();
    // Cpu instructions routines
    void adc(u8 value) noexcept;
    void add(u8 value);
    void addhl(u16 value);
    void addsp(s8 value);
    void z8and(u8 value);
    void bit(u8 bit_number, u8 register_value);
    void call(bool condition, u16 address) noexcept;
    void cp(u8 val);
    void daa();
    void dec(u8 &reg);
    void dec(u8 &high_register, u8 &low_register);
    void dechl();
    void inc(u8 &reg);
    void inc(u8 &high_register, u8 &low_register);
    void inchl();
    void jp(bool condition, u16 address);
    void jr(bool condition, s8 value);
    void ldhl(s8 value);
    void ldnnsp(u16 value);
    void z8or(u8 value);
    void pop(u8 &high_register, u8 &low_register);
    void popaf();
    void push(u16 regs);
    void res(u8 bit_number, u8 &reg);
    void reshl(u8 bit_number);
    void ret(bool condition);
    void rlc(u8 &reg);
    void rlca();
    void rlchl();
    void rl(u8 &reg);
    void rla();
    void rlhl();
    void rrc(u8 &reg);
    void rrca();
    void rrchl();
    void rr(u8 &reg);
    void rra();
    void rrhl();
    void rst(u8 address);
    void sbc(u8 value);
    void set(u8 bit, u8 &reg);
    void sethl(u8 bit_number);
    void sla(u8 &reg);
    void slahl();
    void sra(u8 &reg);
    void srahl();
    void srl(u8 &reg);
    void srlhl();
    void sub(u8 value);
    void swap(u8 &reg);
    void swaphl();
    void z8xor(u8 val);
    void updateIMEFlag();
    void executeOpCode(int a_opCode);
    void executeCBOpCode(u8 a_cbOpCode);
    int checkInterrupts();
    void initRTCTimer();
};

#endif
