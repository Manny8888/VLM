
int fregstate[32];
int subtrace;

enum {
    FPS_LOADT = 1,
    FPS_T,
    FPS_Q,
    FPS_LOADS,
    FPS_S,
    FPS_L,
};

#define SETFLTT(rn, r, s)                                                                                              \
    do {                                                                                                               \
        r = U64FLTT(s);                                                                                                \
        fregstate[rn] = FPS_T;                                                                                         \
    } while (0)

#define LDS(rn, r, s) r = _LDS(rn, s);
#define LDT(rn, r, s) r = _LDT(rn, s);
#define STS(d, rn, r) *d = _STS(rn, r);
#define STT(d, rn, r) *d = _STT(rn, r);

uint64_t _LDS(int regnum, uint64_t /*uint32_t*/ v)
{
    fregstate[regnum] = FPS_LOADS;
    // printf("_LDS: %d %p\n", regnum, v);
    return v;
}

uint64_t _LDT(int regnum, uint64_t v)
{
    fregstate[regnum] = FPS_LOADT;
    return v;
}

uint64_t /*uint32_t*/
_STS(int regnum, uint64_t v)
{
    if (fregstate[regnum] != FPS_L && fregstate[regnum] != FPS_S && fregstate[regnum] != FPS_LOADS) {
        printf("_STS: %p(%d)\n", v, fregstate[regnum]);
    }
    return v;
}

uint64_t _STT(int regnum, uint64_t v)
{
    if (fregstate[regnum] > FPS_T) {
        printf("_STT: %p(%d)\n", v, fregstate[regnum]);
    }
    return v;
}

float fixsfloat(int r, uint64_t v)
{
#if 1
    union {
        double d;
        uint64_t l;
    } u;
    union {
        float f;
        uint64_t l;
    } u2;

    // printf("fixsfloat: %d %p(%d)\n", r, v, fregstate[r]);
    switch (fregstate[r]) {
    case FPS_LOADT:
    case FPS_T:
        u.l = v;
        return (float)u.d;

    case FPS_LOADS:
    case FPS_S:
        u2.l = v;
        return u2.f;
    }

    if (v != 0)
        printf("fixsfloat: f%d is Q/L! %p\n", r, v);
    return 0.0;
#else
    union {
        double d;
        long l;
    } u;
    union {
        float f;
        int i;
        long l;
    } u2;

    if ((v >> 32) == 0) {
        if (fregstate[r] != FPS_S && fregstate[r] != FPS_LOADS && v != 0)
            printf("fixsfloat: assuming S f%d %p(%d)\n", r, v, fregstate[r]);
        u2.l = v;
        return u2.f;
    }

    if (fregstate[r] != FPS_T)
        printf("fixsfloat: assuming T f%d %p(%d)\n", r, v, fregstate[r]);
    u.l = v;
    u2.l = 0;
    u2.f = u.d;

    return u2.f;
#endif
}

double fixtfloat(int r, uint64_t v)
{
#if 1
    union {
        double d;
        uint64_t l;
    } u;
    union {
        float f;
        uint64_t l;
    } u2;

    // printf("fixtfloat: %d %p(%d)\n", r, v, fregstate[r]);
    switch (fregstate[r]) {
    case FPS_LOADT:
    case FPS_T:
        u.l = v;
        return u.d;

    case FPS_LOADS:
    case FPS_S:
        u2.l = v;
        return (double)u2.f;
    }

    printf("fixtfloat: f%d is Q/L! %p(%d)\n", r, v, fregstate[r]);
    return 0.0;
#else
    union {
        double d;
        long l;
    } u;
    union {
        float f;
        int i;
        long l;
    } u2;

    if ((v >> 32) != 0 || v == 0) {
        if (fregstate[r] != FPS_T && v != 0)
            printf("fixtfloat: assuming T f%d %p(%d)\n", r, v, fregstate[r]);
        u.l = v;
        return u.d;
    }

    if (fregstate[r] != FPS_S && fregstate[r] != FPS_LOADS) {
        printf("fixtfloat: assuming S f%d %p(%d)\n", r, v, fregstate[r]);
        while (1)
            ;
    }
    u2.l = v;
    u.d = u2.f;

    return u.d;
#endif
}

#define CPYSN(dr, d, sr1, s1, sr2, s2) d = _CPYSN(dr, sr1, s1, sr2, s2)

uint64_t _CPYSN(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    union {
        double d;
        uint64_t l;
    } u1, u2;
    double signbit = -1.0;
    u1.l = a;
    u2.l = b;

    if (fregstate[ra] != FPS_T || fregstate[rb] != FPS_T) {
        printf("CPYSN: %p %p\n", a, b);
        printf("CPYSN: %g(%d) %g(%d) ", u1.d, fregstate[ra], u2.d, fregstate[rb]);
    }

    if (u1.d <= 0.0)
        signbit = 1.0;
    if (u2.d < 0.0)
        u2.d = -u2.d;
    u2.d *= signbit;
    //  printf(" -> %g\n", u2.d);
    fregstate[rd] = FPS_T;
    return u2.l;
}

#define CVTLQ(dr, d, s1, sr2, s2) d = _CVTLQ(dr, sr2, s2)
#define CVTQL(dr, d, s1, sr2, s2) d = _CVTQL(dr, sr2, s2)
#define CVTQLV(dr, d, s1, sr2, s2) d = _CVTQL(dr, sr2, s2)
#define CVTQS(dr, d, s1, sr2, s2) d = _CVTQS(dr, sr2, s2)
#define CVTQT(dr, d, s1, sr2, s2) d = _CVTQT(dr, sr2, s2)
#define CVTTQ(dr, d, s1, sr2, s2) d = _CVTTQ(dr, sr2, s2)
#define CVTTQV(dr, d, s1, sr2, s2) d = _CVTTQ(dr, sr2, s2)
#define CVTTQVC(dr, d, s1, sr2, s2) d = _CVTTQ(dr, sr2, s2)
#define CVTTQVM(dr, d, s1, sr2, s2) d = _CVTTQVM(dr, sr2, s2)
#define CVTTQSVI(dr, d, s1, sr2, s2) d = _CVTTQ(dr, sr2, s2)
#define CVTTS(dr, d, s1, sr2, s2) d = _CVTTS(dr, sr2, s2)

//#define LOUD

uint64_t _CVTLQ(int dr, int rv2, uint64_t v2)
{
    if (fregstate[rv2] != FPS_L && fregstate[rv2] != FPS_LOADS) {
        printf("_CVTLQ: %p(%d)\n", v2, fregstate[rv2]);
    }
#ifdef LOUD
    if (subtrace)
        printf("_CVTLQ: %p\n", v2);
#endif
    fregstate[dr] = FPS_Q;
    return (int)v2;
}

uint64_t _CVTQL(int dr, int rv2, uint64_t v2)
{
    uint64_t l;
    l = (int)v2;
    fregstate[dr] = FPS_L;
    return l;
}

uint64_t _CVTQS(int dr, int rv2, uint64_t v2)
{
    union {
        float f;
        uint64_t l;
    } u;
    fregstate[dr] = FPS_S;
    u.f = (int64_t) v2;
#ifdef LOUD
    if (subtrace)
        printf("_CVTQS: %p %g -> %p\n", v2, u.f, u.l);
#endif
    return u.l;
}

uint64_t _CVTQT(int dr, int rv2, uint64_t v2)
{
    union {
        double d;
        uint64_t l;
    } u;
    if (fregstate[rv2] != FPS_Q) {
        printf("_CVTQT: %p(%d)\n", v2, fregstate[rv2]);
    }
    fregstate[dr] = FPS_T;
    u.d = (int64_t) v2;
#ifdef LOUD
    //  printf("_CVTQT(d=r%d) %llx %g -> %p\n", dr, v2, u.d, u.l);
    //  printf("_CVTQT(d=r%d) %p %g -> %p\n", dr, v2, u.d, u.l);
    if (subtrace)
        printf("_CVTQT(d=r%d) %p %g -> %p\n", dr, v2, 0.0 /*u.d*/, u.l);
#endif
    return u.l;
}

uint64_t _CVTTQ(int dr, int rv2, uint64_t v2)
{
    union {
        double d;
        uint64_t l;
    } u;
    if (fregstate[rv2] > FPS_T) {
        printf("_CVTTQ: %p(%d)\n", v2, fregstate[rv2]);
    }
    fregstate[dr] = FPS_Q;
    u.l = v2;
    return (uint64_t) u.d;
}

uint64_t _CVTTQVM(int dr, int rv2, uint64_t v2)
{
    union {
        double d;
        uint64_t l;
    } u;
    u.l = v2;
    //  printf("CVTTQVM: %g %d\n", u.d, (int)trunc(u.d));
    //  return (int)trunc(u.d);
    if (u.d < 0.0)
        u.d -= 0.5;
    // if (u.d > 0.0) u.d += 0.5;
    printf("CVTTQVM: %g %d\n", u.d, (int)u.d);
    fregstate[dr] = FPS_Q;
    return (long)u.d;
}

uint64_t _CVTTS(int dr, int rv2, uint64_t v2)
{
    union {
        double d;
        long l;
    } u;
    union {
        float f;
        uint32_t i;
        long l;
    } u2;

    u.l = v2;
    u2.l = 0;
    u2.f = u.d;

    if (fregstate[rv2] != FPS_T && fregstate[rv2] != FPS_LOADT) {
        printf("_CVTTS: f%d %p <- %p(%d)\n", dr, (uint64_t) u2.i, v2, fregstate[rv2]);
    }

    fregstate[dr] = FPS_S;
    return (uint64_t) u2.i;
}

double FLTU64(int rv, uint64_t v) { return fixtfloat(rv, v); }

uint64_t U64FLTT(double v)
{
    union {
        double d;
        uint64_t l;
    } u;
    u.d = v;
    return u.l;
}

uint64_t U64FLTS(float v)
{
    union {
        float f;
        uint64_t l;
    } u;
    //  u.l = 0;
    u.f = v;
    return u.l;
}

#define ADDS(dr, d, sr1, s1, sr2, s2) d = _ADDS(dr, sr1, s1, sr2, s2)
#define SUBS(dr, d, sr1, s1, sr2, s2) d = _SUBS(dr, sr1, s1, sr2, s2)
#define MULS(dr, d, sr1, s1, sr2, s2) d = _MULS(dr, sr1, s1, sr2, s2)
#define DIVS(dr, d, sr1, s1, sr2, s2) d = _DIVS(dr, sr1, s1, sr2, s2)

#define ADDT(dr, d, sr1, s1, sr2, s2) d = _ADDT(dr, sr1, s1, sr2, s2)
#define SUBT(dr, d, sr1, s1, sr2, s2) d = _SUBT(dr, sr1, s1, sr2, s2)
#define MULT(dr, d, sr1, s1, sr2, s2) d = _MULT(dr, sr1, s1, sr2, s2)
#define DIVT(dr, d, sr1, s1, sr2, s2) d = _DIVT(dr, sr1, s1, sr2, s2)

uint64_t _ADDS(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    float fa, fb;
    fa = fixsfloat(ra, a);
    fb = fixsfloat(rb, b);
#ifdef LOUD
    printf("ADDS: %p %p\n", a, b);
    printf("ADDS: %g %g %g\n", fa, fb, fa + fb);
#endif
    fregstate[rd] = FPS_S;
    return U64FLTS(fa + fb);
}

uint64_t _ADDT(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    double fa, fb;
    fa = fixtfloat(ra, a);
    fb = fixtfloat(rb, b);
    fregstate[rd] = FPS_T;
    return U64FLTT(fa + fb);
}

uint64_t _SUBS(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    float fa, fb;
    fa = fixsfloat(ra, a);
    fb = fixsfloat(rb, b);
    fregstate[rd] = FPS_S;
    return U64FLTS(fa - fb);
}

uint64_t _SUBT(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    double fa, fb;
    fa = fixtfloat(ra, a);
    fb = fixtfloat(rb, b);
    fregstate[rd] = FPS_T;
    return U64FLTT(fa - fb);
}

uint64_t _MULS(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    float fa, fb;
    fa = fixsfloat(ra, a);
    fb = fixsfloat(rb, b);
    fregstate[rd] = FPS_S;
    return U64FLTS(fa * fb);
}

uint64_t _MULT(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    double fa, fb;
    fa = fixtfloat(ra, a);
    fb = fixtfloat(rb, b);
    fregstate[rd] = FPS_T;
    return U64FLTT(fa * fb);
}

uint64_t _DIVS(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    float fa, fb;
    fa = fixsfloat(ra, a);
    fb = fixsfloat(rb, b);
#ifdef LOUD
    printf("DIVS: %p %p\n", a, b);
    printf("DIVS: %g %g %g\n", fa, fb, fa / fb);
#endif
    fregstate[rd] = FPS_S;
    return U64FLTS(fa / fb);
}

uint64_t _DIVT(int rd, int ra, uint64_t a, int rb, uint64_t b)
{
    double fa, fb;
    fa = fixtfloat(ra, a);
    fb = fixtfloat(rb, b);
#ifdef LOUD
    printf("DIVT: %d:%p %d:%p -> %p %p\n", fregstate[ra], a, fregstate[rb], b, *(long *)&fa, *(long *)&fb);
    printf("DIVT: %g %g %g %p\n", fa, fb, fa / fb, U64FLTT(fa / fb));
#endif
    fregstate[rd] = FPS_T;
    return U64FLTT(fa / fb);
}

#undef LOUD

/* --- end float --- */
