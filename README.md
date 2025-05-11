# nand
NAND Gate Computer

A computer built from NAND gates, inspired by [https://nandgame.com](https://nandgame.com).

# Computer
The computer supports sixteen 16-bit words of ROM, sixteen words of RAM, and two registers.

The instruction format is:
```
+----------------+----------------------------------+--------------+--------------+
| special        | ALU                              | dest         | condition    |
+----+------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
| ci | halt |  - | sm | -  | u  | o1 | o0 | zx | sw | a  | d  | *a | lt | eq | gt |
+----+------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
```

If `ci` is unset then the remaining 15 bits are stored in register `A`.
Otherwise, the remaining bits are interpreted as follows.

Arithmetic and logic unit (ALU) operates on two inputs `X` and `Y`, and outputs to register `A` and/or register `D` and/or RAM (at address \*A).
* Input `X` is register `D`.
* Input `Y` is register `A` (or \*A if `sm` is set).

Additionally:
* The first input is zeroed if `zx` is set.
* The two inputs are swapped if `sw` is set.

The operations are defined as follows:
```
+----+----+----+-------------+
|  u | o1 | o0 | Description |
+----+----+----+-------------+
|  0 |  0 |  0 | X & Y       |
|  0 |  0 |  1 | X | Y       |
|  0 |  1 |  0 | X ^ Y       |
|  0 |  1 |  1 | ~X          |
|  1 |  0 |  0 | X + Y       |
|  1 |  0 |  1 | X - Y       |
|  1 |  1 |  0 | X + 1       |
|  1 |  1 |  1 | X - 1       |
+----+----+----+-------------+
```

Condition flags can be combined as follows:
```
+----+----+----+-------------+
| lt | eq | gt | Description |
+----+----+----+-------------+
|  0 |  0 |  0 | never       |
|  0 |  0 |  1 | > 0         |
|  0 |  1 |  0 | = 0         |
|  0 |  1 |  1 | ≥ 0         |
|  1 |  0 |  0 | < 0         |
|  1 |  0 |  1 | ≠ 0         |
|  1 |  1 |  0 | ≤ 0         |
|  1 |  1 |  1 | always      |
+----+----+----+-------------+
```

If the condition is met then the `PC` jumps to the location in register `A`.

## Example Program
```
0x0004    A=4
0x8490    D=A         CI | OP_ADD | ZX | DEST_D
0x0003    A=3
0x8715    D--, JNE    CI | OP_DEC | DEST_D | COND_LT | COND_GT
0xc000    HALT        CI | HALT
```
