#include <cstdio>

#include "nand.cpp"

int main()
{
    std::vector<uint16_t> program = {
        /*00*/ 0x0004,
        /*01*/ OP_ADD | ZX | DEST_D, // D = A
        /*02*/ 0x0003,
        /*03*/ OP_DEC | DEST_D | COND_LT | COND_GT, // D--; JNE A
        /*04*/ HALT,
        /*05*/ HALT,
        /*06*/ HALT,
        /*07*/ HALT,
        /*08*/ HALT,
        /*09*/ HALT,
        /*0a*/ HALT,
        /*0b*/ HALT,
        /*0c*/ HALT,
        /*0d*/ HALT,
        /*0e*/ HALT,
        /*0f*/ HALT,
    };

    Signal clk;
    Signal halt;
    Computer g{program, clk, halt};

    while (!halt.get()) {
        // Show state.
        printf("PC:%04x A:%04x D:%04x PA:%04x\n", g.pc(), g.a(), g.d(), g.pa());

        // Clock pulse.
        clk.set(1);
        g.update();
        clk.set(0);
        g.update();
    }
}
