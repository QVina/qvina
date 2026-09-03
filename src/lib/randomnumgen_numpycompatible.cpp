#include "randomnumgen_numpycompatible.hpp"

RandomNumGen_NumpyCompatible_64::RandomNumGen_NumpyCompatible_64(): m_haveNextVal(false), m_nextVal(0){
}

RandomNumGen_NumpyCompatible_64::RandomNumGen_NumpyCompatible_64(uint32_t seed): m_haveNextVal(false), m_nextVal(0), m_mersenneEngine(seed) {
}

void RandomNumGen_NumpyCompatible_64::seed(std::uint_fast32_t newSeed){
    m_mersenneEngine.seed(newSeed);
}

void RandomNumGen_NumpyCompatible_64::discard(unsigned long long z){
    for (unsigned i = 0; i < z; ++i)
        getDouble_UniformDist();  //TODO test which getDouble to use
}

unsigned long long RandomNumGen_NumpyCompatible_64::operator()() {
    unsigned long long upper = ((unsigned long long)m_mersenneEngine()) << 32;
    unsigned long long lower = m_mersenneEngine();
    return (upper | lower);
}

double RandomNumGen_NumpyCompatible_64::getDouble_UniformDist(){
    int a = m_mersenneEngine() >> 5;
    int b = m_mersenneEngine() >> 6;
    return (a * 67108864.0 + b) / 9007199254740992.0;
}

std::uint_fast32_t RandomNumGen_NumpyCompatible_64::random_uint32(std::uint_fast32_t off, std::uint_fast32_t rng) {
    std::uint_fast32_t val, mask = rng;

    if (rng == 0) {
        return 0;
    }

    /* Smallest bit mask >= max */
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;

    while ((val = (m_mersenneEngine() & mask)) > rng)
        ;
    return off + val;
}

double RandomNumGen_NumpyCompatible_64::getDouble_GaussianDist(){
    if (m_haveNextVal) {
        m_haveNextVal = false;
        return m_nextVal;
    }

    double f, x1, x2, r2;
    do {
        int a1 = m_mersenneEngine() >> 5;
        int b1 = m_mersenneEngine() >> 6;
        int a2 = m_mersenneEngine() >> 5;
        int b2 = m_mersenneEngine() >> 6;
        x1 = 2.0 * ((a1 * 67108864.0 + b1) / 9007199254740992.0) - 1.0;
        x2 = 2.0 * ((a2 * 67108864.0 + b2) / 9007199254740992.0) - 1.0;
        r2 = x1 * x1 + x2 * x2;
    } while (r2 >= 1.0 || r2 == 0.0);

    /* Box-Muller transform */
    f = sqrt(-2.0 * log(r2) / r2);
    m_haveNextVal = true;
    m_nextVal = f * x1;
    return f * x2;
}

