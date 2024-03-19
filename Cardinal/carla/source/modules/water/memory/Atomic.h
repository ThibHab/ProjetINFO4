/*
  ==============================================================================

   This file is part of the Water library.
   Copyright (c) 2016 ROLI Ltd.
   Copyright (C) 2017-2023 Filipe Coelho <falktx@falktx.com>

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

  ==============================================================================
*/

#ifndef WATER_ATOMIC_H_INCLUDED
#define WATER_ATOMIC_H_INCLUDED

#include "../water.h"

#ifdef _MSC_VER
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <windows.h>
#endif

#include <stdint.h>

namespace water {

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Weffc++"
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Weffc++"
#elif defined(_MSC_VER)
# pragma warning (push)
# pragma warning (disable: 4311) /* truncation warning */
# ifdef CARLA_OS_64BIT
#  pragma intrinsic (_InterlockedExchange, \
                     _InterlockedExchange64, \
                     _InterlockedExchangeAdd, \
                     _InterlockedExchangeAdd64, \
                     _InterlockedIncrement, \
                     _InterlockedIncrement64, \
                     _InterlockedDecrement, \
                     _InterlockedDecrement64, \
                     _InterlockedCompareExchange, \
                     _InterlockedCompareExchange64, \
                     _ReadWriteBarrier)
# else
#  pragma intrinsic (_InterlockedExchange, \
                     _InterlockedExchangeAdd, \
                     _InterlockedIncrement, \
                     _InterlockedDecrement, \
                     _InterlockedCompareExchange, \
                     _InterlockedCompareExchange64, \
                     _ReadWriteBarrier)
# endif
#endif

#if defined(CARLA_OS_64BIT) || !defined(_MSC_VER)
# define WATER_ATOMIC_64_SUPPORTED
#endif

#ifdef CARLA_OS_64BIT
# define WATER_ALIGN_SIZE 8
#else
# define WATER_ALIGN_SIZE 4
#endif

//==============================================================================
/**
    Simple class to hold a primitive value and perform atomic operations on it.

    The type used must be a 32 or 64 bit primitive, like an int, pointer, etc.
    There are methods to perform most of the basic atomic operations.
*/
template <typename Type>
class Atomic
{
public:
    /** Creates a new value, initialised to zero. */
    inline Atomic() noexcept
        : value (0)
    {
    }

    /** Creates a new value, with a given initial value. */
    inline explicit Atomic (const Type initialValue) noexcept
        : value (initialValue)
    {
    }

    /** Copies another value (atomically). */
    inline Atomic (const Atomic& other) noexcept
        : value (other.get())
    {
    }

    /** Destructor. */
    inline ~Atomic() noexcept
    {
#ifdef CARLA_PROPER_CPP11_SUPPORT
        // This class can only be used for types which are 32 or 64 bits in size.
        static_wassert (sizeof (Type) == 4 || sizeof (Type) == 8);
#endif
    }

    /** Atomically reads and returns the current value. */
    Type get() const noexcept;

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Atomic& other) noexcept         { exchange (other.get()); return *this; }

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Type newValue) noexcept         { exchange (newValue); return *this; }

    /** Atomically sets the current value. */
    void set (Type newValue) noexcept                               { exchange (newValue); }

    /** Atomically sets the current value, returning the value that was replaced. */
    Type exchange (Type value) noexcept;

    /** Atomically adds a number to this value, returning the new value. */
    Type operator+= (Type amountToAdd) noexcept;

    /** Atomically subtracts a number from this value, returning the new value. */
    Type operator-= (Type amountToSubtract) noexcept;

    /** Atomically increments this value, returning the new value. */
    Type operator++() noexcept;

    /** Atomically decrements this value, returning the new value. */
    Type operator--() noexcept;

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        bool compareAndSetBool (Type newValue, Type valueToCompare)
        {
            if (get() == valueToCompare)
            {
                set (newValue);
                return true;
            }

            return false;
        }
        @endcode

        @returns true if the comparison was true and the value was replaced; false if
                 the comparison failed and the value was left unchanged.
        @see compareAndSetValue
    */
    bool compareAndSetBool (Type newValue, Type valueToCompare) noexcept;

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        Type compareAndSetValue (Type newValue, Type valueToCompare)
        {
            Type oldValue = get();
            if (oldValue == valueToCompare)
                set (newValue);

            return oldValue;
        }
        @endcode

        @returns the old value before it was changed.
        @see compareAndSetBool
    */
    Type compareAndSetValue (Type newValue, Type valueToCompare) noexcept;

    /** Implements a memory read/write barrier. */
    static void memoryBarrier() noexcept;

    //==============================================================================
    /** The raw value that this class operates on.
        This is exposed publicly in case you need to manipulate it directly
        for performance reasons.
    */
   #ifdef _MSC_VER
    __declspec (align (WATER_ALIGN_SIZE))
   #else
    __attribute__ ((aligned (WATER_ALIGN_SIZE)))
   #endif
    mutable volatile Type value;

private:
    template <typename Dest, typename Source>
    static inline Dest castTo (Source value) noexcept         { union { Dest d; Source s; } u; u.s = value; return u.d; }

    static inline Type castFrom32Bit (int32 value)  noexcept  { return castTo <Type, int32>  (value); }
    static inline Type castFrom64Bit (int64 value)  noexcept  { return castTo <Type, int64>  (value); }
    static inline Type castFrom32Bit (uint32 value) noexcept  { return castTo <Type, uint32> (value); }
    static inline Type castFrom64Bit (uint64 value) noexcept  { return castTo <Type, uint64> (value); }
    static inline Type castFromLong (long value) noexcept     { return castTo <Type, long>   (value); }
    static inline int32 castTo32Bit (Type value) noexcept     { return castTo <int32, Type>  (value); }
    static inline int64 castTo64Bit (Type value) noexcept     { return castTo <int64, Type>  (value); }
    static inline long castToLong   (Type value) noexcept     { return castTo <long, Type>   (value); }

    Type operator++ (int); // better to just use pre-increment with atomics..
    Type operator-- (int);

    /** This templated negate function will negate pointers as well as integers */
    template <typename ValueType>
    inline ValueType negateValue (ValueType n) noexcept
    {
        return sizeof (ValueType) == 1 ? (ValueType) -(signed char) n
            : (sizeof (ValueType) == 2 ? (ValueType) -(short) n
            : (sizeof (ValueType) == 4 ? (ValueType) -(int) n
            : ((ValueType) -(int64) n)));
    }

    /** This templated negate function will negate pointers as well as integers */
    template <typename PointerType>
    inline PointerType* negateValue (PointerType* n) noexcept
    {
        return reinterpret_cast<PointerType*> (-reinterpret_cast<pointer_sized_int> (n));
    }
};

//==============================================================================
template<>
inline int32 Atomic<int32>::get() const noexcept
{
   #ifdef CARLA_PROPER_CPP11_SUPPORT
    static_wassert (sizeof (int32) == 4);
   #endif
   #ifdef _MSC_VER
    return castFromLong (_InterlockedExchangeAdd (reinterpret_cast<volatile long*> (&value), 0));
   #else
    return castFrom32Bit ((int32) __sync_add_and_fetch (const_cast<volatile int32*> (&value), 0));
   #endif
}

template<>
inline uint32 Atomic<uint32>::get() const noexcept
{
   #ifdef CARLA_PROPER_CPP11_SUPPORT
    static_wassert (sizeof (uint32) == 4);
   #endif
   #ifdef _MSC_VER
    return castFromLong (_InterlockedExchangeAdd (reinterpret_cast<volatile long*> (&value), 0));
   #else
    return castFrom32Bit ((uint32) __sync_add_and_fetch (const_cast<volatile uint32*> (&value), 0));
   #endif
}

#ifdef WATER_ATOMIC_64_SUPPORTED
template<>
inline int64 Atomic<int64>::get() const noexcept
{
   #ifdef CARLA_PROPER_CPP11_SUPPORT
    static_wassert (sizeof (int64) == 8);
   #endif
   #ifdef _MSC_VER
    return castFrom64Bit (_InterlockedExchangeAdd64 (reinterpret_cast<volatile int64*> (&value), 0));
   #else
    return castFrom64Bit ((int64) __sync_add_and_fetch (const_cast<volatile int64*> (&value), 0));
   #endif
}

template<>
inline uint64 Atomic<uint64>::get() const noexcept
{
   #ifdef CARLA_PROPER_CPP11_SUPPORT
    static_wassert (sizeof (uint64) == 8);
   #endif
   #ifdef _MSC_VER
    return castFrom64Bit (_InterlockedExchangeAdd64 (reinterpret_cast<volatile int64*> (&value), 0));
   #else
    return castFrom64Bit ((uint64) __sync_add_and_fetch (const_cast<volatile uint64*> (&value), 0));
   #endif
}
#endif // WATER_ATOMIC_64_SUPPORTED

#ifdef _MSC_VER
template <>
inline int32 Atomic<int32>::exchange (const int32 newValue) noexcept
{
    return castFromLong (_InterlockedExchange (reinterpret_cast<volatile long*> (&value), castToLong (newValue)));
}

template <>
inline uint32 Atomic<uint32>::exchange (const uint32 newValue) noexcept
{
    return castFromLong (_InterlockedExchange (reinterpret_cast<volatile long*> (&value), castToLong (newValue)));
}

template <>
inline int32 Atomic<int32>::operator+= (const int32 amountToAdd) noexcept
{
    return castFromLong (_InterlockedExchangeAdd (reinterpret_cast<volatile long*> (&value), castToLong (amountToAdd)));
}

template <>
inline uint32 Atomic<uint32>::operator+= (const uint32 amountToAdd) noexcept
{
    return castFromLong (_InterlockedExchangeAdd (reinterpret_cast<volatile long*> (&value), castToLong (amountToAdd)));
}

template <>
inline int32 Atomic<int32>::operator++() noexcept
{
    return castFromLong (_InterlockedIncrement (reinterpret_cast<volatile long*> (&value)));
}

template <>
inline uint32 Atomic<uint32>::operator++() noexcept
{
    return castFromLong (_InterlockedIncrement (reinterpret_cast<volatile long*> (&value)));
}

template <>
inline int32 Atomic<int32>::operator--() noexcept
{
    return castFromLong (_InterlockedDecrement (reinterpret_cast<volatile long*> (&value)));
}

template <>
inline uint32 Atomic<uint32>::operator--() noexcept
{
    return castFromLong (_InterlockedDecrement (reinterpret_cast<volatile long*> (&value)));
}

# ifndef CARLA_OS_64BIT
template <>
inline int64 Atomic<int64>::exchange (const int64 newValue) noexcept
{
    return castFrom64Bit (_InterlockedCompareExchange64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (value), castTo64Bit (newValue)));
}

template <>
inline uint64 Atomic<uint64>::exchange (const uint64 newValue) noexcept
{
    return castFrom64Bit (_InterlockedCompareExchange64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (value), castTo64Bit (newValue)));
}
# else
template <>
inline int64 Atomic<int64>::exchange (const int64 newValue) noexcept
{
    return castFrom64Bit (_InterlockedExchange64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (newValue)));
}

template <>
inline uint64 Atomic<uint64>::exchange (const uint64 newValue) noexcept
{
    return castFrom64Bit (_InterlockedExchange64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (newValue)));
}

template <>
inline int64 Atomic<int64>::operator+= (const int64 amountToAdd) noexcept
{
    return castFrom64Bit (_InterlockedExchangeAdd64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (amountToAdd)));
}

template <>
inline uint64 Atomic<uint64>::operator+= (const uint64 amountToAdd) noexcept
{
    return castFrom64Bit (_InterlockedExchangeAdd64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (amountToAdd)));
}

template <>
inline int64 Atomic<int64>::operator++() noexcept
{
    return castFrom64Bit (_InterlockedIncrement64 (reinterpret_cast<volatile int64*> (&value)));
}

template <>
inline uint64 Atomic<uint64>::operator++() noexcept
{
    return castFrom64Bit (_InterlockedIncrement64 (reinterpret_cast<volatile int64*> (&value)));
}

template <>
inline int64 Atomic<int64>::operator--() noexcept
{
    return castFrom64Bit (_InterlockedDecrement64 (reinterpret_cast<volatile int64*> (&value)));
}

template <>
inline uint64 Atomic<uint64>::operator--() noexcept
{
    return castFrom64Bit (_InterlockedDecrement64 (reinterpret_cast<volatile int64*> (&value)));
}
# endif
#else // _MSC_VER
template <typename Type>
inline Type Atomic<Type>::exchange (const Type newValue) noexcept
{
    Type currentVal = value;
    while (! compareAndSetBool (newValue, currentVal)) { currentVal = value; }
    return currentVal;
}

template <typename Type>
inline Type Atomic<Type>::operator+= (const Type amountToAdd) noexcept
{
    return (Type) __sync_add_and_fetch (&value, amountToAdd);
}

template <typename Type>
inline Type Atomic<Type>::operator++() noexcept
{
    return sizeof (Type) == 4 ? (Type) __sync_add_and_fetch (&value, (Type) 1)
                              : (Type) __sync_add_and_fetch ((volatile int64*) &value, 1);
}

template <typename Type>
inline Type Atomic<Type>::operator--() noexcept
{
    return sizeof (Type) == 4 ? (Type) __sync_add_and_fetch (&value, (Type) -1)
                              : (Type) __sync_add_and_fetch ((volatile int64*) &value, -1);
}
#endif // _MSC_VER

template <typename Type>
inline Type Atomic<Type>::operator-= (const Type amountToSubtract) noexcept
{
    return operator+= (negateValue (amountToSubtract));
}

template <typename Type>
inline bool Atomic<Type>::compareAndSetBool (const Type newValue, const Type valueToCompare) noexcept
{
   #ifdef _MSC_VER
    return compareAndSetValue (newValue, valueToCompare) == valueToCompare;
   #else
    return sizeof (Type) == 4 ? __sync_bool_compare_and_swap ((volatile int32*) &value, castTo32Bit (valueToCompare), castTo32Bit (newValue))
                              : __sync_bool_compare_and_swap ((volatile int64*) &value, castTo64Bit (valueToCompare), castTo64Bit (newValue));
   #endif
}

template <typename Type>
inline Type Atomic<Type>::compareAndSetValue (const Type newValue, const Type valueToCompare) noexcept
{
   #ifdef _MSC_VER
    return sizeof (Type) == 4 ? castFromLong (_InterlockedCompareExchange (reinterpret_cast<volatile long*> (&value), castToLong (valueToCompare), castToLong (newValue)))
                              : castFrom64Bit (_InterlockedCompareExchange64 (reinterpret_cast<volatile int64*> (&value), castTo64Bit (valueToCompare), castTo64Bit (newValue)));
   #else
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) __sync_val_compare_and_swap (reinterpret_cast<volatile int32*> (&value), castTo32Bit (valueToCompare), castTo32Bit (newValue)))
                              : castFrom64Bit ((int64) __sync_val_compare_and_swap (reinterpret_cast<volatile int64*> (&value), castTo64Bit (valueToCompare), castTo64Bit (newValue)));
   #endif
}

template <typename Type>
inline void Atomic<Type>::memoryBarrier() noexcept
{
   #ifdef _MSC_VER
    _ReadWriteBarrier();
   #else
    __sync_synchronize();
   #endif
}

#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
# pragma GCC diagnostic pop
#elif defined(_MSC_VER)
# pragma warning (pop)
#endif

}

#endif // WATER_ATOMIC_H_INCLUDED
