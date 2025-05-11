#include "connector.h"
#include "signal.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

typedef SignalN<16> Signal16;
typedef SignalSetN<16> SignalSet16;

/// The fundamental gate, upon which all others are built.
class NandGate : public Gate
{
public:
    Signal& a_;
    Signal& b_;
    Signal& out_;

    NandGate(Signal& a, Signal& b, Signal& out) :
        a_{a},
        b_{b},
        out_{out}
    {
    }

    void update() override
    {
        out_.set(!(a_.get() && b_.get()));
    }
};

class NotGate : public Gate
{
    NandGate nand_;

public:
    NotGate(Signal& in, Signal& out) :
        nand_{in, in, out}
    {
    }

    ~NotGate() override
    {
    }

    void update() override
    {
        nand_.update();
    }
};

class AndGate : public Gate
{
    Signal c_;
    NandGate nand_;
    NotGate not_;

public:
    AndGate(Signal& a, Signal& b, Signal& out) :
        nand_{a, b, c_},
        not_{c_, out}
    {
    }

    ~AndGate() override
    {
    }

    void update() override
    {
        nand_.update();
        not_.update();
    }
};

/// Or.
/// OR = NAND(NAND(A,B), NAND(A,B)) = DeMorgan
class OrGate : public Gate
{
    Signal aprime_;
    NotGate nota_;
    Signal bprime_;
    NotGate notb_;
    NandGate nand_;

public:
    OrGate(Signal& a, Signal& b, Signal& out) :
        nota_{a, aprime_},
        notb_{b, bprime_},
        nand_{aprime_, bprime_, out}
    {
    }

    void update() override
    {
        nota_.update();
        notb_.update();
        nand_.update();
    }
};

class XorGate : public Gate
{
    Signal s1_;
    OrGate or_;
    Signal s2_;
    NandGate nand_;
    AndGate and_;

public:
    XorGate(Signal& a, Signal& b, Signal& out) :
        or_{a, b, s1_},
        nand_{a, b, s2_},
        and_{s1_, s2_, out}
    {
    }

    void update() override
    {
        or_.update();
        nand_.update();
        and_.update();
    }
};

/// Select.
/// Output A if SEL else B.
class SelectGate : public Gate
{
    Signal tmp1_;
    AndGate and1_;
    Signal nsel_;
    NotGate not_;
    Signal tmp2_;
    AndGate and2_;
    OrGate or_;

public:
    SelectGate(Signal& sel, Signal& a, Signal& b, Signal& out) :
        and1_{sel, a, tmp1_},
        not_{sel, nsel_},
        and2_{nsel_, b, tmp2_},
        or_{tmp1_, tmp2_, out}
    {
    }

    ~SelectGate() override
    {
    }

    void update() override
    {
        not_.update();
        and1_.update();
        and2_.update();
        or_.update();
    }
};

template <size_t N>
class NotNGate : public Gate
{
    std::vector<std::unique_ptr<NotGate>> n_;

public:
    NotNGate(SignalN<N>& in, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            n_.push_back(std::make_unique<NotGate>(in.ref(i), out.ref(i)));
        }
    }

    void update() override
    {
        for (auto& n : n_) {
            n->update();
        }
    }
};

template <size_t N>
class AndNGate : public Gate
{
    std::vector<std::unique_ptr<AndGate>> g_;

public:
    AndNGate(SignalN<N>& a, SignalN<N>& b, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            g_.push_back(std::make_unique<AndGate>(a.ref(i), b.ref(i), out.ref(i)));
        }
    }

    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

template <size_t N>
class OrNGate : public Gate
{
    std::vector<std::unique_ptr<OrGate>> g_;

public:
    OrNGate(SignalN<N>& a, SignalN<N>& b, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            g_.push_back(std::make_unique<OrGate>(a.ref(i), b.ref(i), out.ref(i)));
        }
    }

    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

template <size_t N>
class XorNGate : public Gate
{
    std::vector<std::unique_ptr<XorGate>> g_;

public:
    XorNGate(SignalN<N>& a, SignalN<N>& b, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            g_.push_back(std::make_unique<XorGate>(a.ref(i), b.ref(i), out.ref(i)));
        }
    }

    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

/// Select.
/// Output A if not SEL else B.
template <size_t N>
class SelectNGate : public Gate
{
    std::vector<std::unique_ptr<SelectGate>> g_;

public:
    SelectNGate(Signal& sel, SignalN<N>& a, SignalN<N>& b, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            g_.push_back(std::make_unique<SelectGate>(sel, a.ref(i), b.ref(i), out.ref(i)));
        }
    }

    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

/// Mask N bits with single bit.
template <size_t N>
class Mask1xNGate : public Gate
{
    std::vector<std::unique_ptr<AndGate>> g_;

public:
    Mask1xNGate(Signal& a, SignalN<N>& b, SignalN<N>& out)
    {
        for (size_t i = 0; i < N; ++i) {
            g_.push_back(std::make_unique<AndGate>(a, b.ref(i), out.ref(i)));
        }
    }

public:
    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

class Reduce4Gate : public Gate
{
    Signal ab_;
    AndGate and1_;
    Signal cd_;
    AndGate and2_;
    AndGate and_;

public:
    Reduce4Gate(Signal& a, Signal& b, Signal& c, Signal& d, Signal& out) :
        and1_{a, b, ab_},
        and2_{c, d, cd_},
        and_{ab_, cd_, out}
    {
    }

    void update() override
    {
        and1_.update();
        and2_.update();
        and_.update();
    }
};

class Combine16Gate : public Gate
{
    Signal b01_;
    OrGate g01_;
    Signal b23_;
    OrGate g23_;
    Signal b45_;
    OrGate g45_;
    Signal b67_;
    OrGate g67_;
    Signal b89_;
    OrGate g89_;
    Signal bab_;
    OrGate gab_;
    Signal bcd_;
    OrGate gcd_;
    Signal bef_;
    OrGate gef_;
    Signal b0123_;
    OrGate g0123_;
    Signal b4567_;
    OrGate g4567_;
    Signal b89ab_;
    OrGate g89ab_;
    Signal bcdef_;
    OrGate gcdef_;
    Signal b01234567_;
    OrGate g01234567_;
    Signal b89abcdef_;
    OrGate g89abcdef_;
    Signal tmp_;
    OrGate or_;

public:
    Combine16Gate(Signal16& in, Signal& out) :
        g01_{in.ref( 0), in.ref( 1), b01_},
        g23_{in.ref( 2), in.ref( 3), b23_},
        g45_{in.ref( 4), in.ref( 5), b45_},
        g67_{in.ref( 6), in.ref( 7), b67_},
        g89_{in.ref( 8), in.ref( 9), b89_},
        gab_{in.ref(10), in.ref(11), bab_},
        gcd_{in.ref(12), in.ref(13), bcd_},
        gef_{in.ref(14), in.ref(15), bef_},

        g0123_{b01_, b23_, b0123_},
        g4567_{b45_, b67_, b4567_},
        g89ab_{b89_, bab_, b89ab_},
        gcdef_{bcd_, bef_, bcdef_},

        g01234567_{b0123_, b4567_, b01234567_},
        g89abcdef_{b89ab_, bcdef_, b89abcdef_},

        or_{b01234567_, b89abcdef_, out}
    {
    }

    void update() override
    {
        g01_.update();
        g23_.update();
        g45_.update();
        g67_.update();
        g89_.update();
        gab_.update();
        gcd_.update();
        gef_.update();

        g0123_.update();
        g4567_.update();
        g89ab_.update();
        gcdef_.update();

        g01234567_.update();
        g89abcdef_.update();

        or_.update();
    }
};

/// Decoder.
/// Takes a 4-bit address and outputs 16 lines, where only one is high at a time — the one corresponding to the binary value of the input.
class Decoder4to16Gate : public Gate
{
    Signal n0_;
    Signal n1_;
    Signal n2_;
    Signal n3_;
    NotGate not0_;
    NotGate not1_;
    NotGate not2_;
    NotGate not3_;

    Reduce4Gate and0_;
    Reduce4Gate and1_;
    Reduce4Gate and2_;
    Reduce4Gate and3_;
    Reduce4Gate and4_;
    Reduce4Gate and5_;
    Reduce4Gate and6_;
    Reduce4Gate and7_;
    Reduce4Gate and8_;
    Reduce4Gate and9_;
    Reduce4Gate and10_;
    Reduce4Gate and11_;
    Reduce4Gate and12_;
    Reduce4Gate and13_;
    Reduce4Gate and14_;
    Reduce4Gate and15_;

public:
    Decoder4to16Gate(Signal16& in, Signal16& out) :
        not0_{in.ref(0), n0_},
        not1_{in.ref(1), n1_},
        not2_{in.ref(2), n2_},
        not3_{in.ref(3), n3_},

        and0_ {n3_,       n2_,       n1_,       n0_,       out.ref( 0)},
        and1_ {n3_,       n2_,       n1_,       in.ref(0), out.ref( 1)},
        and2_ {n3_,       n2_,       in.ref(1), n0_,       out.ref( 2)},
        and3_ {n3_,       n2_,       in.ref(1), in.ref(0), out.ref( 3)},
        and4_ {n3_,       in.ref(2), n1_,       n0_,       out.ref( 4)},
        and5_ {n3_,       in.ref(2), n1_,       in.ref(0), out.ref( 5)},
        and6_ {n3_,       in.ref(2), in.ref(1), n0_,       out.ref( 6)},
        and7_ {n3_,       in.ref(2), in.ref(1), in.ref(0), out.ref( 7)},
        and8_ {in.ref(3), n2_,       n1_,       n0_,       out.ref( 8)},
        and9_ {in.ref(3), n2_,       n1_,       in.ref(0), out.ref( 9)},
        and10_{in.ref(3), n2_,       in.ref(1), n0_,       out.ref(10)},
        and11_{in.ref(3), n2_,       in.ref(1), in.ref(0), out.ref(11)},
        and12_{in.ref(3), in.ref(2), n1_,       n0_,       out.ref(12)},
        and13_{in.ref(3), in.ref(2), n1_,       in.ref(0), out.ref(13)},
        and14_{in.ref(3), in.ref(2), in.ref(1), n0_,       out.ref(14)},
        and15_{in.ref(3), in.ref(2), in.ref(1), in.ref(0), out.ref(15)}
    {
    }

    void update() override
    {
        not0_.update();
        not1_.update();
        not2_.update();
        not3_.update();

        and0_.update();
        and1_.update();
        and2_.update();
        and3_.update();
        and4_.update();
        and5_.update();
        and6_.update();
        and7_.update();
        and8_.update();
        and9_.update();
        and10_.update();
        and11_.update();
        and12_.update();
        and13_.update();
        and14_.update();
        and15_.update();
    }
};

/// Multiplexer.
/// out = in[ad]
class Mux16to1Gate : public Gate
{
    SignalSet16 hot_;
    Decoder4to16Gate decoder_;
    SignalSet16 anded_;
    AndNGate<16> mask_;
    Combine16Gate combine_;

public:
    Mux16to1Gate(Signal16& in, Signal16& ad, Signal& out) :
        decoder_{ad, hot_},
        mask_{in, hot_, anded_},
        combine_{anded_, out}
    {
    }

    void update() override
    {
        decoder_.update();
        mask_.update();
        combine_.update();
    }
};

class DataLatchGate : public Gate
{
    /// The initial output (before @c st is set for the first time) is unspecified.
    SelectGate mux_;

public:
    DataLatchGate(Signal& st, Signal& d, Signal& out) :
        mux_{st, d, out, out}
    {
    }

    void update() override
    {
        mux_.update();
    }
};

/// DataFlipFlip.
/// Inputs @c st and @c d may change when clock @c clk is low.
/// When @c clk changes to high, then the current value of @c d is stored.
/// When @c clk changes to low again, then the previously stored value is output.
class DataFlipFlop : public Gate
{
    Signal tmp1_;
    AndGate and_;
    Signal tmp2_;
    DataLatchGate l2_;
    Signal nclk_;
    NotGate not_;
    DataLatchGate l1_;

public:
    DataFlipFlop(Signal& st, Signal& in, Signal& clk, Signal& out) :
        and_{st, clk, tmp1_},
        l2_{tmp1_, in, tmp2_},
        not_{clk, nclk_},
        l1_{nclk_, tmp2_, out}
    {
    }

    ~DataFlipFlop() override
    {
    }

    void update() override
    {
        and_.update();
        l2_.update();
        not_.update();
        l1_.update();
    }
};

// 16-bit register.
class Register : public Gate
{
    std::vector<std::unique_ptr<DataFlipFlop>> g_;

public:
    Register(Signal& st, Signal16& in, Signal& clk, Signal16& out)
    {
        for (size_t i = 0; i < in.size(); ++i) {
            g_.push_back(std::make_unique<DataFlipFlop>(st, in.ref(i), clk, out.ref(i)));
        }
    }

    ~Register() override
    {
    }

    void update() override
    {
        for (auto& g : g_) {
            g->update();
        }
    }
};

class HalfAdderGate : public Gate
{
    AndGate and_;
    XorGate xor_;

public:
    HalfAdderGate(Signal& a, Signal& b, Signal& h, Signal& l) :
        and_{a, b, h},
        xor_{a, b, l}
    {
    }

    void update() override
    {
        and_.update();
        xor_.update();
    }
};

class FullAdderGate : public Gate
{
    Signal h1_;
    Signal l1_;
    HalfAdderGate g1_;
    Signal h2_;
    HalfAdderGate g2_;
    OrGate or_;

public:
    FullAdderGate(Signal& a, Signal& b, Signal& c, Signal& h, Signal& l) :
        g1_{a, b, h1_, l1_},
        g2_{l1_, c, h2_, l},
        or_{h1_, h2_, h}
    {
    }

    void update() override
    {
        g1_.update();
        g2_.update();
        or_.update();
    }
};

class Add16Gate : public Gate
{
    Signal h0_;
    FullAdderGate f0_;
    Signal h1_;
    FullAdderGate f1_;
    Signal h2_;
    FullAdderGate f2_;
    Signal h3_;
    FullAdderGate f3_;
    Signal h4_;
    FullAdderGate f4_;
    Signal h5_;
    FullAdderGate f5_;
    Signal h6_;
    FullAdderGate f6_;
    Signal h7_;
    FullAdderGate f7_;
    Signal h8_;
    FullAdderGate f8_;
    Signal h9_;
    FullAdderGate f9_;
    Signal h10_;
    FullAdderGate f10_;
    Signal h11_;
    FullAdderGate f11_;
    Signal h12_;
    FullAdderGate f12_;
    Signal h13_;
    FullAdderGate f13_;
    Signal h14_;
    FullAdderGate f14_;
    FullAdderGate f15_;

public:
    Add16Gate(Signal16& a, Signal16& b, Signal& c_in, Signal16& s, Signal& c_out) :
        f0_ {a.ref( 0), b.ref( 0), c_in, h0_,   s.ref( 0)},
        f1_ {a.ref( 1), b.ref( 1), h0_,  h1_,   s.ref( 1)},
        f2_ {a.ref( 2), b.ref( 2), h1_,  h2_,   s.ref( 2)},
        f3_ {a.ref( 3), b.ref( 3), h2_,  h3_,   s.ref( 3)},
        f4_ {a.ref( 4), b.ref( 4), h3_,  h4_,   s.ref( 4)},
        f5_ {a.ref( 5), b.ref( 5), h4_,  h5_,   s.ref( 5)},
        f6_ {a.ref( 6), b.ref( 6), h5_,  h6_,   s.ref( 6)},
        f7_ {a.ref( 7), b.ref( 7), h6_,  h7_,   s.ref( 7)},
        f8_ {a.ref( 8), b.ref( 8), h7_,  h8_,   s.ref( 8)},
        f9_ {a.ref( 9), b.ref( 9), h8_,  h9_,   s.ref( 9)},
        f10_{a.ref(10), b.ref(10), h9_,  h10_,  s.ref(10)},
        f11_{a.ref(11), b.ref(11), h10_, h11_,  s.ref(11)},
        f12_{a.ref(12), b.ref(12), h11_, h12_,  s.ref(12)},
        f13_{a.ref(13), b.ref(13), h12_, h13_,  s.ref(13)},
        f14_{a.ref(14), b.ref(14), h13_, h14_,  s.ref(14)},
        f15_{a.ref(15), b.ref(15), h14_, c_out, s.ref(15)}
    {
    }

    void update() override
    {
        f0_.update();
        f1_.update();
        f2_.update();
        f3_.update();
        f4_.update();
        f5_.update();
        f6_.update();
        f7_.update();
        f8_.update();
        f9_.update();
        f10_.update();
        f11_.update();
        f12_.update();
        f13_.update();
        f14_.update();
        f15_.update();
    }
};

class Sub16Gate : public Gate
{
    SignalSet16 b_inv_;
    NotNGate<16> inv_;

    Signal zero_;
    Signal one_;
    NandGate nand_;

    Signal c_;
    Add16Gate add_;

public:
    Sub16Gate(Signal16& a, Signal16& b, Signal16& out) :
        inv_{b, b_inv_},
        nand_{zero_, zero_, one_},
        add_{a, b_inv_, one_, out, c_}
    {
    }

    void update() override
    {
        inv_.update();
        nand_.update();
        add_.update();
    }
};

class Inc16Gate : public Gate
{
    Signal zero_;
    Signal one_;
    NandGate nand_;
    SignalSet16 zero16_;
    Signal c_;
    Add16Gate add_;

public:
    Inc16Gate(Signal16& in, Signal16& out) :
        nand_{zero_, zero_, one_},
        add_{zero16_, in, one_, out, c_}
    {
    }

    void update() override
    {
        nand_.update();
        add_.update();
    }
};

class Counter : public Gate
{
    Signal zero_;
    Signal one_;
    NandGate nand_;
    SignalSet16 a_;
    Inc16Gate inc_;
    SignalSet16 tmp_;
    SelectNGate<16> mux_;
    Register reg_;

public:
    Counter(Signal& sel, Signal16& x, Signal& clk, Signal16& out) :
        nand_{zero_, zero_, one_},
        inc_{out, a_},
        mux_{sel, x, a_, tmp_},
        reg_{one_, tmp_, clk, out}
    {
    }

    void update() override
    {
        nand_.update();
        inc_.update();
        mux_.update();
        reg_.update();
    }
};

/// Logic Unit.
/// 00 X&Y
/// 01 X|Y
/// 10 X^Y
/// 11 ~X
class LogicUnit : public Gate
{
    SignalSet16 s1_;
    AndNGate<16> and_;
    SignalSet16 s2_;
    OrNGate<16> or_;
    SignalSet16 s3_;
    SelectNGate<16> select1_;

    SignalSet16 s4_;
    XorNGate<16> xor_;
    SignalSet16 s5_;
    NotNGate<16> not_;
    SignalSet16 s6_;
    SelectNGate<16> select2_;

    SelectNGate<16> select_;

public:
    LogicUnit(Signal& op1, Signal& op0, Signal16& x, Signal16& y, Signal16& out) :
        and_{x, y, s1_},
        or_{x, y, s2_},
        select1_{op0, s2_, s1_, s3_},

        xor_{x, y, s4_},
        not_{x, s5_},
        select2_{op0, s5_, s4_, s6_},

        select_{op1, s6_, s3_, out}
    {
    }

    void update() override
    {
        and_.update();
        or_.update();
        select1_.update();

        not_.update();
        xor_.update();
        select2_.update();

        select_.update();
    }
};

/// ArithmeticUnit.
/// 00 X+Y
/// 01 X-Y
/// 10 X+1
/// 11 X-1
class ArithmeticUnit : public Gate
{
    SignalSet16 xy_add_;
    Signal c1_;
    Add16Gate add_xy_;
    SignalSet16 xy_sub_;
    Sub16Gate sub_xy_;
    SignalSet16 xy_;
    SelectNGate<16> select1_;

    Signal zero_;
    SignalSet16 one_;
    NandGate nand_;

    SignalSet16 x1_add_;
    Signal c2_;
    Add16Gate add_x1_;
    SignalSet16 x1_sub_;
    Sub16Gate sub_x1_;
    SignalSet16 x1_;
    SelectNGate<16> select2_;

    SelectNGate<16> select_;

public:
    ArithmeticUnit(Signal& op1, Signal& op0, Signal16& x, Signal16& y, Signal16& out) :
        add_xy_{x, y, zero_, xy_add_, c1_},
        sub_xy_{x, y, xy_sub_},
        select1_{op0, xy_sub_, xy_add_, xy_},

        nand_{zero_, zero_, one_.ref(0)},
        add_x1_{x, one_, zero_, x1_add_, c2_},
        sub_x1_{x, one_, x1_sub_},
        select2_{op0, x1_sub_, x1_add_, x1_},

        select_{op1, x1_, xy_, out}
    {
    }

    void update() override
    {
        add_xy_.update();
        sub_xy_.update();
        select1_.update();

        nand_.update();
        add_x1_.update();
        sub_x1_.update();
        select2_.update();

        select_.update();
    }
};

class ArithmeticAndLogicUnit : public Gate
{
    SignalSet16 tmp_lhs_;
    SelectNGate<16> select_xy_;

    SignalSet16 zero_;
    SignalSet16 lhs_;
    SelectNGate<16> select_zx_;

    SignalSet16 rhs_;
    SelectNGate<16> select_yx_;

    SignalSet16 logic_output_;
    LogicUnit logic_;

    SignalSet16 arith_output_;
    ArithmeticUnit arith_;

    SelectNGate<16> select_;

public:
    ArithmeticAndLogicUnit(Signal& u, Signal& op1, Signal& op0, Signal& zx, Signal& sw, Signal16& x, Signal16& y, Signal16& out) :
        select_xy_{sw, y, x, tmp_lhs_},

        select_zx_{zx, zero_, tmp_lhs_, lhs_},

        select_yx_{sw, x, y, rhs_},

        logic_{op1, op0, lhs_, rhs_, logic_output_},

        arith_{op1, op0, lhs_, rhs_, arith_output_},

        select_{u, arith_output_, logic_output_, out}
    {
    }

    void update() override
    {
        select_xy_.update();
        select_zx_.update();
        select_yx_.update();
        logic_.update();
        arith_.update();
        select_.update();
    }
};

class IsZeroGate : public Gate
{
    Signal combined_;
    Combine16Gate combine_;
    NotGate not_;

public:
    IsZeroGate(Signal16& in, Signal& out) :
        combine_{in, combined_},
        not_{combined_, out}
    {
    }

    void update() override
    {
        combine_.update();
        not_.update();
    }
};

class IsNegativeGate : public Gate
{
    Connector connect_;

public:
    IsNegativeGate(Signal16& in, Signal& out) :
        connect_{in.ref(15), out}
    {
    }

    void update() override
    {
        connect_.update();
    }
};

/// ConditionUnit.
/// The flags can be combined so:
/// lt eq gt
///  0  0  0  never
///  0  0  1  X > 0
///  0  1  0  X = 0
///  0  1  1  X ≥ 0
///  1  0  0  X < 0
///  1  0  1  X ≠ 0
///  1  1  0  X ≤ 0
///  1  1  1  always
class ConditionUnit : public Gate
{
    Signal is_lt_;
    IsNegativeGate lt_gate_;
    Signal condition_lt_;
    AndGate and_lt_;

    Signal is_eq_;
    IsZeroGate eq_gate_;
    Signal condition_eq_;
    AndGate and_eq_;

    Signal condition_lt_eq_;
    OrGate sub_or_;

    Signal c1_;
    NotGate not1_;
    Signal c2_;
    NotGate not2_;
    Signal is_gt_;
    AndGate sub_and_;

    Signal condition_gt_;
    AndGate and_gt_;
    OrGate or_;

public:
    ConditionUnit(Signal& lt, Signal& eq, Signal& gt, Signal16& x, Signal& out) :
        lt_gate_{x, is_lt_},
        and_lt_{lt, is_lt_, condition_lt_},

        eq_gate_{x, is_eq_},
        and_eq_{eq, is_eq_, condition_eq_},

        sub_or_{condition_lt_, condition_eq_, condition_lt_eq_},

        not1_{is_lt_, c1_},
        not2_{is_eq_, c2_},
        sub_and_{c1_, c2_, is_gt_},

        and_gt_{gt, is_gt_, condition_gt_},
        or_{condition_lt_eq_, condition_gt_, out}
    {
    }

    void update() override
    {
        lt_gate_.update();
        and_lt_.update();
        eq_gate_.update();
        and_eq_.update();
        sub_or_.update();
        not1_.update();
        not2_.update();
        sub_and_.update();
        and_gt_.update();
        or_.update();
    }
};

/*
 * Machine based on https://nandgame.com
 * (Retrieved 2024-05-03.)
 *
 * Instruction Format
 *
 * +----------------+----------------------------------+--------------+--------------+
 * | special        | ALU                              | dest         | condition    |
 * +----+------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | ci | halt |  - | sm | -  | u  | o1 | o0 | zx | sw | a  | d  | *a | lt | eq | gt |
 * +----+------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * halt: extension
 *
 * ALU:
 * - X = register D
 * - Y = sm ? *A : A
 *
 * u o1 o0  description
 * --------------------
 * 0 0 0    X & Y
 * 0 0 1    X | Y
 * 0 1 0    X ^ Y
 * 0 1 1    ~X
 * 1 0 0    X + Y
 * 1 0 1    X - Y
 * 1 1 0    X + 1
 * 1 1 1    X - 1
 *
 * zx: lhs=0
 *
 * sw: X and Y are swapped
 */

// bit 15
const uint16_t CI       = 0x8000;

// bit 14 (extension)
const uint16_t HALT     = (CI | 0x4000);

// bit 12
const uint16_t SM       = (CI | 0x1000);

// bits 10..8
const uint16_t OP_AND   = (CI | (0x0 << 8));
const uint16_t OP_OR    = (CI | (0x1 << 8));
const uint16_t OP_XOR   = (CI | (0x2 << 8));
const uint16_t OP_NOT   = (CI | (0x3 << 8));
const uint16_t OP_ADD   = (CI | (0x4 << 8));
const uint16_t OP_SUB   = (CI | (0x5 << 8));
const uint16_t OP_INC   = (CI | (0x6 << 8));
const uint16_t OP_DEC   = (CI | (0x7 << 8));

// bits 7..6
const uint16_t ZX       = (CI | 0x0080);
const uint16_t SW       = (CI | 0x0040);

// bits 5..3
const uint16_t DEST_A   = (CI | 0x0020);
const uint16_t DEST_D   = (CI | 0x0010);
const uint16_t DEST_PA  = (CI | 0x0008);

// bits 2..0
const uint16_t COND_LT  = (CI | 0x0004);
const uint16_t COND_EQ  = (CI | 0x0002);
const uint16_t COND_GT  = (CI | 0x0001);
const uint16_t ALWAYS   = (CI | 0x0007);

class AluInstruction : public Gate
{
    SignalSet16 y_;
    SelectNGate<16> select_;
    ArithmeticAndLogicUnit alu_;
    ConditionUnit cond_;
    Connector connect_sel_a_;
    Connector connect_sel_d_;
    Connector connect_sel_pa_;

public:
    AluInstruction(Signal16& instr, Signal16& a, Signal16& d, Signal16& pa, Signal16& r, Signal& sel_a, Signal& sel_d, Signal& sel_pa, Signal& j) :
        select_{instr.ref(12), pa, a, y_},
        alu_{instr.ref(10), instr.ref(9), instr.ref(8), instr.ref(7), instr.ref(6), d, y_, r},
        cond_{instr.ref(2), instr.ref(1), instr.ref(0), r, j},
        connect_sel_a_{instr.ref(5), sel_a},
        connect_sel_d_{instr.ref(4), sel_d},
        connect_sel_pa_{instr.ref(3), sel_pa}
    {
    }

    void update() override
    {
        select_.update();
        alu_.update();
        cond_.update();
        connect_sel_a_.update();
        connect_sel_d_.update();
        connect_sel_pa_.update();
    }
};

class ControlSelectorGate : public Gate
{
    SelectNGate<16> choose_r_;
    SelectGate choose_a_;
    SelectGate choose_d_;
    SelectGate choose_pa_;
    SelectGate choose_j_;

public:
    ControlSelectorGate(Signal& s, Signal16& r1, Signal& a1, Signal& d1, Signal& pa1, Signal& j1, Signal16& r0, Signal& a0, Signal& d0, Signal& pa0, Signal& j0, Signal16& r, Signal& a, Signal& d, Signal& pa, Signal& j) :
        choose_r_ {s, r1,  r0,  r},
        choose_a_ {s, a1,  a0,  a},
        choose_d_ {s, d1,  d0,  d},
        choose_pa_{s, pa1, pa0, pa},
        choose_j_ {s, j1,  j0,  j}
    {
    }

    void update() override
    {
        choose_r_.update();
        choose_a_.update();
        choose_d_.update();
        choose_pa_.update();
        choose_j_.update();
    }
};

class ControlUnit : public Gate
{
    SignalSet16 r1_;
    Signal sel_a1_;
    Signal sel_d1_;
    Signal sel_pa1_;
    Signal sel_j1_;
    AluInstruction alu_;

    Signal zero_;
    Signal one_;
    NandGate nand_;

    ControlSelectorGate selector_;

public:
    ControlUnit(Signal16& instr, Signal16& a, Signal16& d, Signal16& pa, Signal16& r, Signal& sel_a, Signal& sel_d, Signal& sel_pa, Signal& j) :
        alu_{instr, a, d, pa, r1_, sel_a1_, sel_d1_, sel_pa1_, sel_j1_},

        nand_{zero_, zero_, one_},

        selector_{instr.ref(15), r1_, sel_a1_, sel_d1_, sel_pa1_, sel_j1_, instr, one_, zero_, zero_, zero_, r, sel_a, sel_d, sel_pa, j}
    {
    }

    void update() override
    {
        alu_.update();
        nand_.update();
        selector_.update();
    }
};

class Ram16x16 : public Gate
{
    SignalSet16 hot_;
    Decoder4to16Gate decoder_;
    SignalSet16 select_;
    Mask1xNGate<16> mask_;
    std::vector<std::unique_ptr<SignalSet16>> rout_;
    std::vector<std::unique_ptr<Gate>> gates_;
    std::vector<std::unique_ptr<Signal16>> slices_;

public:
    Ram16x16(Signal& st, Signal16& x, Signal16& ad, Signal& clk, Signal16& out) :
        decoder_{ad, hot_},
        mask_{st, hot_, select_}
    {
        // 16 registers of 16-bits.
        for (size_t reg = 0; reg < 16; ++reg) {
            auto tmp = std::make_unique<SignalSet16>();
            gates_.push_back(std::make_unique<Register>(select_.ref(reg), x, clk, *tmp));
            rout_.push_back(std::move(tmp));
        }

        // For each bit, an instance of Mux16to1Gate is used to select the register-specific bit.
        for (size_t bit = 0; bit < 16; ++bit) {
            auto slice = std::make_unique<Signal16>();
            for (size_t reg = 0; reg < 16; ++reg) {
                slice->setptr(reg, rout_[reg]->ptr(bit));
            }
            gates_.push_back(std::make_unique<Mux16to1Gate>(*slice, ad, out.ref(bit)));
            slices_.push_back(std::move(slice));
        }
    }

    ~Ram16x16() override
    {
    }

    void update() override
    {
        decoder_.update();
        mask_.update();
        for (auto& g : gates_) {
            g->update();
        }
    }
};

/// Combined memory unit.
/// Two 16-bit registers called A and D, and a RAM unit.
class CombinedMemoryUnit : public Gate
{
    Register ra_;
    Register rd_;
    Ram16x16 ram_;

public:
    CombinedMemoryUnit(Signal& sel_a, Signal& sel_d, Signal& sel_pa, Signal16& x, Signal& clk, Signal16& a, Signal16& d, Signal16 &pa) :
        ra_{sel_a, x, clk, a},
        rd_{sel_d, x, clk, d},
        ram_{sel_pa, x, a, clk, pa}
    {
    }

    void update() override
    {
        ra_.update();
        rd_.update();
        ram_.update();
    }
};

/// ROM.
/// Not clocked.
class Rom16x16 : public Gate
{
    std::vector<std::unique_ptr<SignalSet16>> rom_;
    std::vector<std::unique_ptr<Signal16>> slices_;
    std::vector<std::unique_ptr<Gate>> gates_;

public:
    Rom16x16(std::vector<uint16_t>& program, Signal16& ad, Signal16& out)
    {
	// ROM is modelled as 16x16 constant signals.
        for (size_t address = 0; address < 16; ++address) {
	    auto s = std::make_unique<SignalSet16>();
	    s->setint(program[address]);
	    rom_.push_back(std::move(s));
        }

        // For each bit, an instance of Mux16to1Gate is used to select the address-specific bit.
        for (size_t bit = 0; bit < 16; ++bit) {
            auto slice = std::make_unique<Signal16>();
            for (size_t address = 0; address < 16; ++address) {
                slice->setptr(address, rom_[address]->ptr(bit));
            }
            gates_.push_back(std::make_unique<Mux16to1Gate>(*slice, ad, out.ref(bit)));
            slices_.push_back(std::move(slice));
        }
    }

    void update() override
    {
        for (auto& g : gates_) {
            g->update();
        }
    }
};

/// Computer.
/// Each clock cycle changes the program counter depending on j.
class Computer : public Gate
{
    Signal j_;
    SignalSet16 a_;
    SignalSet16 pc_;
    Counter counter_;

    SignalSet16 instr_;
    Rom16x16 rom_;

    SignalSet16 d_;
    SignalSet16 pa_;
    SignalSet16 r_;
    Signal sel_a_;
    Signal sel_d_;
    Signal sel_pa_;
    ControlUnit control_;
    Connector connect_;

    CombinedMemoryUnit memory_;

public:
    Computer(std::vector<uint16_t>& program, Signal& clk, Signal& halt) :
        counter_{j_, a_, clk, pc_},

        rom_{program, pc_, instr_},

        control_{instr_, a_, d_, pa_, r_, sel_a_, sel_d_, sel_pa_, j_},
        connect_{instr_.ref(14), halt},

        memory_{sel_a_, sel_d_, sel_pa_, r_, clk, a_, d_, pa_}
    {
    }

    uint16_t pc() const
    {
        return pc_.getint();
    }

    uint16_t a() const
    {
        return a_.getint();
    }

    uint16_t d() const
    {
        return d_.getint();
    }

    uint16_t pa() const
    {
        return pa_.getint();
    }

    void update() override
    {
        rom_.update();
        control_.update();
        memory_.update();
        counter_.update();
        connect_.update();
    }
};
