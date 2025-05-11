#include <cassert>
#include <cstdio>

#include "nand.cpp"

static void test_fundamental()
{
    {
        Signal in;
        Signal out;
        Connector g{in, out};
        g.update();
        assert(out.get() == 0);
        in.set(1);
        g.update();
        assert(out.get() == 1);
        in.set(0);
        g.update();
        assert(out.get() == 0);
    }

    {
        Signal a;
        Signal b;
        Signal out;
        NandGate g{a, b, out};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned expect_out)
        {
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 1);
        test(0, 1, 1);
        test(1, 0, 1);
        test(1, 1, 0);
    }

    {
        Signal zero;
        SignalSet16 one;
        NandGate nand{zero, zero, one.ref(0)};
        nand.update();
        assert(one.getint() == 1);
    }

    {
        Signal a;
        Signal out;
        NotGate g{a, out};
        auto test = [&](unsigned a_value, unsigned expect_out)
        {
            a.set(a_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 1);
        test(1, 0);
    }

    {
        Signal a;
        Signal b;
        Signal out;
        AndGate g{a, b, out};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned expect_out)
        {
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 0);
        test(0, 1, 0);
        test(1, 0, 0);
        test(1, 1, 1);
    }

    {
        Signal a;
        Signal b;
        Signal out;
        OrGate g{a, b, out};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned expect_out)
        {
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 0);
        test(0, 1, 1);
        test(1, 0, 1);
        test(1, 1, 1);
    }

    {
        Signal a;
        Signal b;
        Signal out;
        XorGate g{a, b, out};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned expect_out)
        {
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 0);
        test(0, 1, 1);
        test(1, 0, 1);
        test(1, 1, 0);
    }

    {
        Signal sel;
        Signal a;
        Signal b;
        Signal out;
        SelectGate g{sel, a, b, out};
        auto test = [&](unsigned sel_value, unsigned a_value, unsigned b_value, unsigned expect_out)
        {
            sel.set(sel_value);
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(1, 1, 0, 1);
        test(0, 1, 0, 0);

        test(1, 0, 1, 0);
        test(0, 0, 1, 1);
    }
}

static void test_fundamental_multi()
{
    {
        SignalSet16 s;
        assert(s.size() == 16);
        s.setint(5);
        assert(s.get(0) == 1);
        assert(s.get(1) == 0);
        assert(s.get(2) == 1);
        assert(s.get(3) == 0);
        assert(s.getint() == 5);
        s.setint(10);
        assert(s.get(0) == 0);
        assert(s.get(1) == 1);
        assert(s.get(2) == 0);
        assert(s.get(3) == 1);
        assert(s.getint() == 10);
    }

    {
        SignalSet16 in;
        SignalSet16 out;
        NotNGate<16> g{in, out};
        in.setint(0xaaaa);
        g.update();
        assert(out.getint() == 0x5555);
    }

    {
        SignalSet16 a;
        SignalSet16 b;
        SignalSet16 out;
        AndNGate<16> g{a, b, out};
        a.setint(0xaaaa);
        b.setint(0x05af);
        g.update();
        assert(out.getint() == 0x00aa);
    }

    {
        SignalSet16 a;
        SignalSet16 b;
        SignalSet16 out;
        OrNGate<16> g{a, b, out};
        a.setint(0xaa00);
        b.setint(0x0180);
        g.update();
        assert(out.getint() == 0xab80);
    }

    {
        SignalSet16 a;
        SignalSet16 b;
        SignalSet16 out;
        XorNGate<16> g{a, b, out};
        a.setint(0xaaaa);
        b.setint(0x05ab);
        g.update();
        assert(out.getint() == 0xaf01);
    }

    {
        Signal sel;
        SignalSet16 a;
        SignalSet16 b;
        SignalSet16 out;
        SelectNGate<16> g{sel, a, b, out};
        auto test = [&](unsigned sel_value, uint16_t a_value, uint16_t b_value, uint16_t expect_out)
        {
            sel.set(sel_value);
            a.setint(a_value);
            b.setint(b_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(1, 0x1234, 0x5678, 0x1234);
        test(0, 0x1234, 0x5678, 0x5678);
    }

    {
        Signal a;
        SignalSetN<3> b;
        SignalSetN<3> out;
        Mask1xNGate<3> g{a, b, out};
        b.setint(5);
        g.update();
        assert(out.getint() == 0);
        a.set(1);
        g.update();
        assert(out.getint() == 5);
    }

    {
        Signal a;
        Signal b;
        Signal c;
        Signal d;
        Signal out;
        Reduce4Gate g{a, b, c, d, out};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned c_value, unsigned d_value, unsigned expect_out)
        {
            a.set(a_value);
            b.set(b_value);
            c.set(c_value);
            d.set(d_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 0, 0, 0);
        test(0, 0, 0, 1, 0);
        test(0, 0, 1, 0, 0);
        test(0, 0, 1, 1, 0);
        test(0, 1, 0, 0, 0);
        test(0, 1, 0, 1, 0);
        test(0, 1, 1, 0, 0);
        test(0, 1, 1, 1, 0);
        test(1, 0, 0, 0, 0);
        test(1, 0, 0, 1, 0);
        test(1, 0, 1, 0, 0);
        test(1, 0, 1, 1, 0);
        test(1, 1, 0, 0, 0);
        test(1, 1, 0, 1, 0);
        test(1, 1, 1, 0, 0);
        test(1, 1, 1, 1, 1);
    }

    {
        SignalSet16 in;
        Signal out;
        Combine16Gate g{in, out};

        g.update();
        assert(out.get() == 0);

        for (uint16_t i = 0; i < 16; ++i) {
            in.setint(static_cast<uint16_t>(1 << i));
            g.update();
            assert(out.get() == 1);
        }
    }

    {
        SignalSet16 in;
        SignalSet16 out;
        Decoder4to16Gate g{in, out};

        for (uint16_t i = 0; i < 16; ++i) {
            in.setint(i);
            g.update();
            assert(out.getint() == (1 << i));
        }
    }

    {
        SignalSet16 in;
        SignalSet16 ad;
        Signal out;
        Mux16to1Gate g{in, ad, out};
        auto test = [&](uint16_t in_value, uint16_t ad_value, unsigned expected_out)
        {
	    in.setint(in_value);
	    ad.setint(ad_value);
	    g.update();
	    assert(out.get() == expected_out);
        };

	test(0x40      | 0x10, 5, 0);
	test(     0x20,        5, 1);
    }
}

static void test_latch()
{
    {
        Signal sel;
        Signal d;
        Signal out;
        DataLatchGate g{sel, d, out};
        auto test = [&](unsigned sel_value, unsigned d_value, unsigned expect_out)
        {
            sel.set(sel_value);
            d.set(d_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 0);
        test(0, 1, 0);
        test(1, 0, 0);
        test(1, 1, 1);

        test(0, 1, 1);
        test(0, 0, 1);
    }

    {
        Signal st;
        Signal d;
        Signal clk;
        Signal out;
        DataFlipFlop g{st, d, clk, out};
        auto test = [&](unsigned st_value, unsigned d_value, unsigned clk_value, unsigned expect_out)
        {
            st.set(st_value);
            d.set(d_value);
            clk.set(clk_value);
            g.update();
            assert(out.get() == expect_out);
        };

        test(0, 0, 1, 0);
        test(0, 0, 0, 0);

        test(0, 1, 1, 0);
        test(0, 1, 0, 0);

        test(1, 1, 1, 0);
        test(1, 1, 0, 1);

        test(1, 0, 1, 1);
        test(1, 0, 0, 0);
    }

    {
        Signal st;
        SignalSet16 in;
        Signal clk;
        SignalSet16 out;
        Register g{st, in, clk, out};
        auto test = [&](unsigned st_value, uint16_t in_value, unsigned clk_value, uint16_t expect_out)
        {
            st.set(st_value);
            in.setint(in_value);
            clk.set(clk_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 5, 0, 0);
        test(1, 5, 1, 0);
        test(1, 5, 0, 5);

        test(0, 6, 0, 5);
        test(1, 6, 1, 5);
        test(1, 6, 0, 6);
    }
}

static void test_adder()
{
    {
        Signal a;
        Signal b;
        Signal h;
        Signal l;
        HalfAdderGate g{a, b, h, l};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned expect_h, unsigned expect_l)
        {
            a.set(a_value);
            b.set(b_value);
            g.update();
            assert(h.get() == expect_h);
            assert(l.get() == expect_l);
        };

        test(0, 0, 0, 0);
        test(0, 1, 0, 1);
        test(1, 0, 0, 1);
        test(1, 1, 1, 0);
    }

    {
        Signal a;
        Signal b;
        Signal c;
        Signal h;
        Signal l;
        FullAdderGate g{a, b, c, h, l};
        auto test = [&](unsigned a_value, unsigned b_value, unsigned c_value, unsigned expect_h, unsigned expect_l)
        {
            a.set(a_value);
            b.set(b_value);
            c.set(c_value);
            g.update();
            assert(h.get() == expect_h);
            assert(l.get() == expect_l);
        };

        test(0, 0, 0, 0, 0);
        test(0, 0, 1, 0, 1);
        test(0, 1, 0, 0, 1);
        test(0, 1, 1, 1, 0);
        test(1, 0, 0, 0, 1);
        test(1, 0, 1, 1, 0);
        test(1, 1, 0, 1, 0);
        test(1, 1, 1, 1, 1);
    }

    {
        SignalSet16 a;
        SignalSet16 b;
        Signal   c;
        SignalSet16 out;
        Signal   c_out;
        Add16Gate g{a, b, c, out, c_out};
        auto test = [&](uint16_t a_value, uint16_t b_value, uint16_t expect_out, unsigned expect_c_out)
        {
            a.setint(a_value);
            b.setint(b_value);
            g.update();
            assert(a.getint() == a_value);
            assert(b.getint() == b_value);
            assert(out.getint() == expect_out);
            assert(c_out.get() == expect_c_out);
        };

        test(0, 0, 0, 0);
        test(0, 1, 1, 0);
        test(1, 0, 1, 0);

        test(0xfffc, 1, 0xfffd, 0);
        test(0xfffd, 1, 0xfffe, 0);
        test(0xffff, 1, 0x0000, 1);
        test(0x0000, 1, 0x0001, 0);
        test(0x0001, 1, 0x0002, 0);

        test(65535, 65535, 65534, 1);
    }

    {
        SignalSet16 a;
        SignalSet16 b;
        SignalSet16 out;
        Sub16Gate g{a, b, out};
        auto test = [&](uint16_t a_value, uint16_t b_value, uint16_t expect_out)
        {
            a.setint(a_value);
            b.setint(b_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 0, 0);
        test(1, 0, 1);
        test(1, 1, 0);
        test(1, 2, 0xffff);
        test(1, 3, 0xfffe);
        test(2, 2, 0);
        test(4, 2, 2);
        test(0x8000, 0x8000, 0);
        test(0x8000, 0x8002, 0xfffe);
    }

    {
        SignalSet16 in;
        SignalSet16 out;
        Inc16Gate g{in, out};
        auto test = [&](uint16_t in_value, uint16_t expect_out)
        {
            in.setint(in_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0xfffc, 0xfffd);
        test(0xfffd, 0xfffe);
        test(0xffff, 0x0000);
        test(0x0000, 0x0001);
        test(0x0001, 0x0002);
        test(0x0002, 0x0003);
    }

    {
        Signal sel;
        SignalSet16 in;
        Signal clk;
        SignalSet16 out;
        Counter g{sel, in, clk, out};
        auto test = [&](unsigned sel_value, uint16_t in_value, unsigned clk_value, uint16_t expect_out)
        {
            sel.set(sel_value);
            in.setint(in_value);
            clk.set(clk_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 0, 0, 0);

        test(0, 0, 1, 0);
        test(0, 0, 0, 1);
        test(0, 0, 1, 1);
        test(0, 0, 0, 2);
        test(0, 0, 1, 2);
        test(0, 0, 0, 3);

        test(1, 65534, 1, 3);
        test(0, 65534, 0, 65534);

        test(0, 0, 1, 65534);
        test(0, 0, 0, 65535);
        test(0, 0, 1, 65535);
        test(0, 0, 0, 0);
        test(0, 0, 1, 0);
        test(0, 0, 0, 1);
    }
}

static void test_alu()
{
    {
        Signal op1;
        Signal op0;
        SignalSet16 x;
        SignalSet16 y;
        SignalSet16 out;
        LogicUnit g{op1, op0, x, y, out};
        auto test = [&](unsigned op1_value, unsigned op0_value, uint16_t x_value, uint16_t y_value, uint16_t expect_out)
        {
            op1.set(op1_value);
            op0.set(op0_value);
            x.setint(x_value);
            y.setint(y_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 0, 0,      0xffff, 0);
        test(0, 0, 0xaaaa, 0x05af, 0x00aa);
        test(0, 1, 0xaa00, 0x0180, 0xab80);
        test(1, 0, 0xaaaa, 0x05ab, 0xaf01);
        test(1, 1, 0xaaaa, 0,      0x5555);
    }

    {
        Signal op1;
        Signal op0;
        SignalSet16 x;
        SignalSet16 y;
        SignalSet16 out;
        ArithmeticUnit g{op1, op0, x, y, out};
        auto test = [&](unsigned op1_value, unsigned op0_value, uint16_t x_value, uint16_t y_value, uint16_t expect_out)
        {
            op1.set(op1_value);
            op0.set(op0_value);
            x.setint(x_value);
            y.setint(y_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 0, 7, 4, 11);
        test(0, 1, 7, 4,  3);
        test(1, 0, 7, 4,  8);
        test(1, 1, 7, 4,  6);

        test(1, 0, 0xfffc, 3, 0xfffd);
        test(1, 0, 0xfffd, 3, 0xfffe);
        test(1, 0, 0xfffe, 3, 0xffff);
        test(1, 0, 0xffff, 3, 0x0000);
        test(1, 0, 0x0000, 3, 0x0001);
        test(1, 0, 0x0001, 3, 0x0002);
    }

    {
	Signal u;
	Signal op1;
	Signal op0;
	Signal zx;
	Signal sw;
	SignalSet16 x;
	SignalSet16 y;
	SignalSet16 out;
	ArithmeticAndLogicUnit g{u, op1, op0, zx, sw, x, y, out};
        auto test = [&](unsigned u_value, unsigned op1_value, unsigned op0_value, unsigned zx_value, unsigned sw_value, uint16_t x_value, uint16_t y_value, uint16_t expect_out)
        {
            u.set(u_value);
            op1.set(op1_value);
            op0.set(op0_value);
            zx.set(zx_value);
            sw.set(sw_value);
            x.setint(x_value);
            y.setint(y_value);
            g.update();
            assert(out.getint() == expect_out);
        };

        test(0, 0, 0, 0, 0, 0, 0, 0);
        test(1, 0, 0, 0, 0, 7, 4, 11);
        test(1, 0, 1, 0, 0, 7, 4, 3);
        test(1, 0, 1, 0, 1, 7, 4, 0xfffd);
        test(1, 0, 1, 1, 0, 7, 4, 0xfffc);
        test(1, 0, 1, 1, 1, 7, 4, 0xfff9);

        test(0, 0, 0, 0, 0, 0, 0, 0);
        test(0, 0, 0, 0, 0, 0, 0xffff, 0);
        test(0, 1, 0, 0, 0, 0xaaaa, 0x05ab, 0xaf01);
    }

    {
        SignalSet16 a;
        Signal out;
        IsZeroGate g{a, out};
        g.update();
        assert(out.get() == 1);
        a.setint(0x1000);
        g.update();
        assert(out.get() == 0);
    }

    {
        SignalSet16 a;
        Signal out;
        IsNegativeGate g{a, out};
        g.update();
        assert(out.get() == 0);
        a.setint(32768);
        g.update();
        assert(out.get() == 1);
    }

    {
        Signal lt;
        Signal eq;
        Signal gt;
        SignalSet16 x;
        Signal out;
        ConditionUnit g{lt, eq, gt, x, out};
        auto test = [&](unsigned lt_value, unsigned eq_value, unsigned gt_value)
        {
            lt.set(lt_value);
            eq.set(eq_value);
            gt.set(gt_value);

            x.setint(65535);
            g.update();
            assert(out.get() == lt.get());

            x.setint(0);
            g.update();
            assert(out.get() == eq.get());

            x.setint(1);
            g.update();
            assert(out.get() == gt.get());
        };

        test(0, 0, 0);
        test(0, 0, 1);
        test(0, 1, 0);
        test(0, 1, 1);
        test(1, 0, 0);
        test(1, 0, 1);
        test(1, 1, 0);
        test(1, 1, 1);
    }
}

static void test_control_unit()
{
    SignalSet16 instr;
    SignalSet16 a;
    SignalSet16 d;
    SignalSet16 pa;
    SignalSet16 r;
    Signal sel_a;
    Signal sel_d;
    Signal sel_pa;
    Signal j;
    ControlUnit g{instr, a, d, pa, r, sel_a, sel_d, sel_pa, j};
    auto test = [&](uint16_t instr_value, uint16_t a_value, uint16_t d_value, uint16_t pa_value, uint16_t expect_r, unsigned expect_sel_a, unsigned expect_sel_d, unsigned expect_sel_pa, unsigned expect_j)
    {
        instr.setint(instr_value);
        a.setint(a_value);
        d.setint(d_value);
        pa.setint(pa_value);
        g.update();
        assert(r.getint() == expect_r);
        assert(sel_a.get() == expect_sel_a);
        assert(sel_d.get() == expect_sel_d);
        assert(sel_pa.get() == expect_sel_pa);
        assert(j.get() == expect_j);
    };

    // D = 0
    test(0  | OP_AND | ZX | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x0000, 0, 1, 0, 0);
    // D = 1
    test(0  | OP_INC | ZX | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x0001, 0, 1, 0, 0);
    // D = A
    test(0  | OP_ADD | ZX | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x1111, 0, 1, 0, 0);
    // D = *A
    test(SM | OP_ADD | ZX | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x3333, 0, 1, 0, 0);

    // D = D+1
    test(0  | OP_INC | 0  | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x2223, 0, 1, 0, 0);
    // D = D-1
    test(0  | OP_DEC | 0  | 0  | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x2221, 0, 1, 0, 0);

    // D = A+1
    test(0  | OP_INC | 0  | SW | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x1112, 0, 1, 0, 0);
    // D = *A+1
    test(SM | OP_INC | 0  | SW | DEST_D  | 0,       0x1111, 0x2222, 0x3333, 0x3334, 0, 1, 0, 0);

    // A = 0
    test(0x0000,                                   0x1111, 0x2222, 0x3333, 0x0000, 1, 0, 0, 0);
    // A = 1
    test(0x0001,                                   0x1111, 0x2222, 0x3333, 0x0001, 1, 0, 0, 0);

    // A = 0
    test(0  | OP_AND | ZX | 0  | DEST_A  | 0,       0x1111, 0x2222, 0x3333, 0x0000, 1, 0, 0, 0);
    // A = D
    test(0  | OP_ADD | ZX | SW | DEST_A  | 0,       0x1111, 0x2222, 0x3333, 0x2222, 1, 0, 0, 0);
    // A = D-1
    test(0  | OP_DEC | 0  | 0  | DEST_A  | 0,       0x1111, 0x2222, 0x3333, 0x2221, 1, 0, 0, 0);
    // A = A-1
    test(0  | OP_DEC | 0  | SW | DEST_A  | 0,       0x1111, 0x2222, 0x3333, 0x1110, 1, 0, 0, 0);

    // *A = D
    test(0  | OP_ADD | ZX | SW | DEST_PA | 0,       0x1111, 0x2222, 0x3333, 0x2222, 0, 0, 1, 0);

    // D&A
    test(0  | OP_AND | 0  | 0  | 0       | 0,       0x1111, 0x3210, 0x3333, 0x1010, 0, 0, 0, 0);
    // D|A
    test(0  | OP_OR  | 0  | 0  | 0       | 0,       0x1010, 0x0123, 0x3333, 0x1133, 0, 0, 0, 0);
    // D^A
    test(0  | OP_XOR | 0  | 0  | 0       | 0,       0x1010, 0x2012, 0x3333, 0x3002, 0, 0, 0, 0);
    // ~D
    test(0  | OP_NOT | 0  | 0  | 0       | 0,       0x1111, 0xa5a5, 0x3333, 0x5a5a, 0, 0, 0, 0);

    // COND(D-A) JLT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_LT, 0x0003, 0x000a, 0x3333, 0x0007, 0, 0, 0, 0);
    // COND(D-A) JEQ
    test(0  | OP_SUB | 0  | 0  | 0       | COND_EQ, 0x0003, 0x000a, 0x3333, 0x0007, 0, 0, 0, 0);
    // COND(D-A) JGT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_GT, 0x0003, 0x000a, 0x3333, 0x0007, 0, 0, 0, 1);

    // COND(D-A) JLT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_LT, 0x000a, 0x000a, 0x3333, 0x0000, 0, 0, 0, 0);
    // COND(D-A) JEQ
    test(0  | OP_SUB | 0  | 0  | 0       | COND_EQ, 0x000a, 0x000a, 0x3333, 0x0000, 0, 0, 0, 1);
    // COND(D-A) JGT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_GT, 0x000a, 0x000a, 0x3333, 0x0000, 0, 0, 0, 0);

    // COND(D-A) JLT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_LT, 0x000a, 0x0003, 0x3333, 0xfff9, 0, 0, 0, 1);
    // COND(D-A) JEQ
    test(0  | OP_SUB | 0  | 0  | 0       | COND_EQ, 0x000a, 0x0003, 0x3333, 0xfff9, 0, 0, 0, 0);
    // COND(D-A) JGT
    test(0  | OP_SUB | 0  | 0  | 0       | COND_GT, 0x000a, 0x0003, 0x3333, 0xfff9, 0, 0, 0, 0);

    // JMP
    test(0  | 0      | 0  | 0  | 0       | ALWAYS,  0x1111, 0x2222, 0x3333, 0x0000, 0, 0, 0, 1);
}

static void test_memory()
{
    {
        Signal st;
        SignalSet16 x;
        SignalSet16 ad;
        Signal clk;
        SignalSet16 out;
        Ram16x16 g{st, x, ad, clk, out};
        auto test = [&](unsigned st_value, uint16_t x_value, uint16_t ad_value, unsigned clk_value, uint16_t expected_out)
        {
	    st.set(st_value);
	    x.setint(x_value);
	    ad.setint(ad_value);
	    clk.set(clk_value);
	    g.update();
	    assert(out.getint() == expected_out);
        };

	test(1, 42, 2, 1, 0);
	test(1, 42, 2, 0, 42);

	test(1, 69, 3, 1, 0);
	test(1, 69, 3, 0, 69);

	test(0,  0, 2, 1, 42);
	test(0,  0, 2, 0, 42);

	test(0,  0, 3, 1, 69);
	test(0,  0, 3, 0, 69);
    }

    {
        Signal sel_a;
        Signal sel_d;
        Signal sel_pa;
        SignalSet16 x;
        Signal clk;
        SignalSet16 a;
        SignalSet16 d;
        SignalSet16 pa;
        CombinedMemoryUnit g{sel_a, sel_d, sel_pa, x, clk, a, d, pa};
	auto test = [&](unsigned sel_a_value, unsigned sel_d_value, unsigned sel_pa_value, uint16_t x_value, unsigned clk_value, uint16_t expected_a, uint16_t expected_d, uint16_t expected_pa)
	{
	    sel_a.set(sel_a_value);
	    sel_d.set(sel_d_value);
	    sel_pa.set(sel_pa_value);
	    x.setint(x_value);
	    clk.set(clk_value);
	    g.update();
	    assert(a.getint() == expected_a);
	    assert(d.getint() == expected_d);
	    assert(pa.getint() == expected_pa);
	};

	test(0, 0, 0, 0, 0, 0, 0, 0);
	test(1, 0, 1, 2, 1, 0, 0, 0);
	test(1, 0, 1, 2, 0, 2, 0, 0);

        // Second clock pulse for store to PA to use address in register A.
	test(1, 0, 1, 2, 1, 2, 0, 0);
	test(1, 0, 1, 2, 0, 2, 0, 2);

	test(0, 1, 0, 0xabcd, 1, 2, 0,      2);
	test(0, 1, 0, 0xabcd, 0, 2, 0xabcd, 2);

	// Test wraparound at 18 = 16 + 2 (since CombinedMemory uses Ram16x16).
	test(1, 0, 0, 18,     1, 2,  0xabcd, 2);
	test(1, 0, 0, 18,     0, 18, 0xabcd, 2);
    }
}

int main()
{
    test_fundamental();
    test_fundamental_multi();
    test_latch();
    test_adder();
    test_alu();
    test_control_unit();
    test_memory();
}
