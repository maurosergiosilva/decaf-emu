#pragma once
#include "bitutils.h"
#include "decaf_assert.h"
#include "fixed.h"
#include <spdlog/fmt/fmt.h>
#include <type_traits>

template<typename BitfieldType, typename ValueType, unsigned Position, unsigned Bits>
struct BitfieldHelper
{
   static const auto MinValue = static_cast<ValueType>(0);
   static const auto MaxValue = static_cast<ValueType>((1ull << (Bits)) - 1);
   static const auto Mask = (static_cast<typename BitfieldType::StorageType>((1ull << (Bits)) - 1)) << (Position);

   static constexpr ValueType get(BitfieldType bitfield)
   {
      return static_cast<ValueType>((bitfield.value & Mask) >> (Position));
   }

   static inline BitfieldType set(BitfieldType bitfield, ValueType value)
   {
      decaf_assert(value >= MinValue, fmt::format("{} >= {}", value, static_cast<unsigned>(MinValue)));
      decaf_assert(value <= MaxValue, fmt::format("{} <= {}", value, static_cast<unsigned>(MaxValue)));
      bitfield.value &= ~Mask;
      bitfield.value |= static_cast<typename BitfieldType::StorageType>(value) << (Position);
      return bitfield;
   }
};

// Specialise for bool because of compiler warnings for static_cast<bool>(int)
template<typename BitfieldType, unsigned Position, unsigned Bits>
struct BitfieldHelper<BitfieldType, bool, Position, Bits>
{
   static const auto Mask = (static_cast<typename BitfieldType::StorageType>((1ull << (Bits)) - 1)) << (Position);

   static constexpr bool get(BitfieldType bitfield)
   {
      return !!((bitfield.value & Mask) >> (Position));
   }

   static inline BitfieldType set(BitfieldType bitfield, bool value)
   {
      bitfield.value &= ~Mask;
      bitfield.value |= (static_cast<typename BitfieldType::StorageType>(value ? 1 : 0)) << (Position);
      return bitfield;
   }
};

// Specialise for fixed_point
template<typename BitfieldType, unsigned Position, unsigned Bits, class Rep, int Exponent>
struct BitfieldHelper<BitfieldType, sg14::fixed_point<Rep, Exponent>, Position, Bits>
{
   using FixedType = sg14::fixed_point<Rep, Exponent>;
   using ValueBitfield = BitfieldHelper<BitfieldType, Rep, Position, Bits>;

   static FixedType get(BitfieldType bitfield)
   {
      auto value = ValueBitfield::get(bitfield);

      // fixed_point internal format expects sign extended value
      if (std::is_signed<Rep>::value) {
         value = sign_extend<Bits, Rep>(value);
      }

      return FixedType::from_data(value);
   }

   static inline BitfieldType set(BitfieldType bitfield, FixedType fixedValue)
   {
      // fixed_point internal format is sign extended so we must & off the extra bits
      auto value = fixedValue.data() & static_cast<Rep>(((1ull << Bits) - 1));
      return ValueBitfield::set(bitfield, value);
   }
};

#ifndef DECAF_USE_STDLAYOUT_BITFIELD

#define BITFIELD(Name, Type) \
   union Name \
   { \
      using BitfieldType = Name; \
      using StorageType = Type; \
      Type value; \
      static inline Name get(Type v) { \
         Name bitfield; \
         bitfield.value = v; \
         return bitfield; \
      }

#define BITFIELD_ENTRY(Pos, Size, ValueType, Name) \
   private: struct { StorageType : Pos; StorageType _##Name : Size; }; \
   public: inline ValueType Name() const { \
      return BitfieldHelper<BitfieldType, ValueType, Pos, Size>::get(*this); \
   } \
   inline BitfieldType Name(ValueType fieldValue) const { \
      return BitfieldHelper<BitfieldType, ValueType, Pos, Size>::set(*this, fieldValue); \
   }

#else

#define BITFIELD(Name, Type) \
   struct Name \
   { \
      using BitfieldType = Name; \
      using StorageType = Type; \
      Type value; \
      static inline Name get(Type v) { \
         Name bitfield; \
         bitfield.value = v; \
         return bitfield; \
      }

#define BITFIELD_ENTRY(Pos, Size, ValueType, Name) \
   inline ValueType Name() const { \
      return BitfieldHelper<BitfieldType, ValueType, Pos, Size>::get(*this); \
   } \
   inline BitfieldType Name(ValueType fieldValue) const { \
      return BitfieldHelper<BitfieldType, ValueType, Pos, Size>::set(*this, fieldValue); \
   }

#endif

#define BITFIELD_END \
   };
