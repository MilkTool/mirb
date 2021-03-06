#pragma once
#include "class.hpp"
#include "../number.hpp"
#include "../context.hpp"

namespace Mirb
{
	class Bignum:
		public Object
	{
		private:
			static value_t to_s(Bignum *self, intptr_t base);
			static value_t neg(Bignum *obj);
			static value_t add(Bignum *obj, value_t other);
			static value_t sub(Bignum *obj, value_t other);
			static value_t mul(Bignum *obj, value_t other);
			static value_t div(Bignum *obj, value_t other);
			static value_t mod(Bignum *obj, value_t other);
			static value_t compare(Bignum *obj, value_t other);
			static value_t zero(Bignum *obj);
			static value_t coerce(Bignum *obj, value_t other);
			static value_t pow(Bignum *obj, intptr_t other);
			static value_t lshift(Bignum *obj, intptr_t shift);
			static value_t rshift(Bignum *obj, intptr_t shift);
			static value_t bitwise_and(Bignum *obj, value_t other);
			static value_t bitwise_xor(Bignum *obj, value_t other);
			static value_t bitwise_or(Bignum *obj, value_t other);

		public:
			static value_t to_string(const Number &number, intptr_t base);

			Number number;

			Bignum(intptr_t value) : Object(Type::Bignum, context->bignum_class), number(value) {}
			Bignum(size_t value) : Object(Type::Bignum, context->bignum_class), number(value) {}
			Bignum(const Number &value) : Object(Type::Bignum, context->bignum_class), number(value) {}
			Bignum(Number &&value) : Object(Type::Bignum, context->bignum_class), number(value) {}
			
			static const bool finalizer = true;

			static void initialize();
	};
};
