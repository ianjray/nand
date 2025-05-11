#pragma once

#include <cstdint>
#include <vector>

/// A signal describes the inputs and outputs of gates.
class Signal
{
    unsigned value_;

public:
    Signal() : value_{}
    {
    }

    /// Get signal value.
    unsigned get() const
    {
        return value_;
    }

    /// Set signal value.
    void set(unsigned value)
    {
        value_ = (value != 0 ? 1 : 0);
    }
};

/// Interface to set of N signals.
template <size_t N>
class SignalN
{
    std::vector<Signal*> array_;

public:
    /// Construct empty set.
    SignalN()
    {
        for (size_t i = 0; i < N; ++i) {
            array_.push_back(nullptr);
        }
    }

    // Construct with pre-defined set.
    SignalN(std::vector<Signal> &storage) : SignalN<N>()
    {
        for (size_t i = 0; i < N; ++i) {
            array_[i] = &storage[i];
        }
    }

    /// @return Number of elements.
    size_t size() const
    {
        return N;
    }

    /// Get value.
    unsigned get(size_t i) const
    {
        return array_[i]->get();
    }

    /// Get reference.
    Signal& ref(size_t i)
    {
        return *array_[i];
    }

    /// Get pointer.
    Signal* ptr(size_t i)
    {
        return array_[i];
    }

    /// Set pointer.
    void setptr(size_t i, Signal* s)
    {
        array_[i] = s;
    }

    /// Convert to integer for simulation purposes.
    uint16_t getint() const
    {
        uint16_t x{};
        unsigned shift{};
        for (const auto& s : array_) {
            x |= (s->get() << shift);
            shift++;
        }
        return x;
    }

    /// Set value from integer for simulation purposes.
    void setint(uint16_t x)
    {
        for (auto& s : array_) {
            s->set(x & 1);
            x >>= 1;
        }
    }
};

/// Concrete set of N signals.
template <size_t N>
class SignalSetN : public SignalN<N>
{
    std::vector<Signal> array_;

public:
    /// Construct set of size @c N.
    SignalSetN() : SignalN<N>(), array_{N, Signal{}}
    {
        for (size_t i = 0; i < N; ++i) {
            this->setptr(i, &array_[i]);
        }
    }
};
