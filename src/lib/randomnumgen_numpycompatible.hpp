#ifndef RANDOMNUMGEN_NUMPYCOMPATIBLE_H
#define RANDOMNUMGEN_NUMPYCOMPATIBLE_H

#include <random>
#include <stdexcept>
#include <climits>

//Uniform distribution - numpy.rand
class RandomNumGen_NumpyCompatible_64 {
public:
    // Lets callers written against boost/std engines construct one
    // unchanged: `rng generator(static_cast<rng::result_type>(seed))`.
    typedef std::uint_fast32_t result_type;

    RandomNumGen_NumpyCompatible_64();
    RandomNumGen_NumpyCompatible_64(uint32_t newSeed);

    std::uint_fast32_t min() const { return m_mersenneEngine.min(); }
    std::uint_fast32_t max() const { return m_mersenneEngine.max(); }
    void seed(std::uint_fast32_t seed);
    void discard(unsigned long long);      // NOTE!!  Advances and discards twice as many values as passed in to keep tracking with Numpy order
    unsigned long long operator()();
    double getDouble_UniformDist();                    //Calculates the next uniformly random double as numpy.rand does
    double getDouble_GaussianDist();                    //Calculates the next normally (Gaussian) distrubuted random double as numpy.randn does


    std::uint_fast32_t random_uint32(std::uint_fast32_t off, std::uint_fast32_t rng);

    std::string getGeneratorType() const { return "RandomNumGen_NumpyCompatible"; }

    static std::uint_fast32_t convert_to_uint_fast32_t(size_t what) {
        if (what > UINT_MAX) {
            throw std::invalid_argument("incompatible data types: size_t, std::uint_fast32_t");
        }
        return static_cast<std::uint_fast32_t>(what);
    }

private:
    bool m_haveNextVal;
    double m_nextVal = NULL;
    std::mt19937 m_mersenneEngine;
};

#endif
