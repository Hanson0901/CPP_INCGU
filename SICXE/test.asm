COPY    START   1000
FIRST   LDA     ALPHA
        ADD     ONE
        STA     BETA
        LDCH    CHARZ
        STCH    COUT
        LDB     GAMMA
        LDX     DELTA
        STX     OUTX
        SUB     TWO
        MUL     THREE
        DIV     FOUR
        COMP    FIVE
        JEQ     EQUAL
        J       NOTEQ
EQUAL   JSUB    SUBRTN
        J       ENDPROG
NOTEQ   RSUB
SUBRTN  LDA     #99
        RSUB
ENDPROG RSUB
        ORG     1060
ALPHA   WORD    5
ONE     WORD    1
TWO     WORD    2
THREE   WORD    3
FOUR    WORD    4
FIVE    WORD    5
BETA    RESW    1
GAMMA   WORD    7
DELTA   WORD    8
OUTX    RESW    1
CHARZ   BYTE    C'Z'
COUT    RESB    1
        END     FIRST