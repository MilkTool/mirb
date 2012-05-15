#include "range.hpp"
#include "fixnum.hpp"
#include "../runtime.hpp"

namespace Mirb
{
	value_t Range::to_s(Range *obj)
	{
		OnStack<1> os(obj);

		CharArray low = inspect_obj(obj->low);

		OnStackString<1> oss(low);
		
		CharArray high = inspect_obj(obj->high);

		return (low + (const char_t *)(obj->flag ? "..." : "..") + high).to_string();
	}
	
	bool Range::convert_to_index(size_t &start, size_t &length, size_t size)
	{
		if(!Value::is_fixnum(low) || !Value::is_fixnum(high))
		{
			raise(context->runtime_error, "Expected Fixnum range attributes");
			return false;
		}
		
		int left = Fixnum::to_int(low);
		int right = Fixnum::to_int(high);
		
		if(left < 0)
		{
			left = -left;

			if((size_t)left > size)
				left = 0;
			else
				left = size - (size_t)left;
		}

		if(right < 0)
		{
			right = -right;

			if((size_t)right > size)
				right = 0;
			else
				right = size - (size_t)right;
		}

		if((size_t)left >= size || left > right)
		{
			start = 0;
			length = 0;

			return true;
		}
		else
		{
			start = left;
			length = right - left + (flag ? 0 : 1);

			if(start + length > size)
				length = size - start;

			return true;
		}

	}

	Range *Range::allocate(value_t low, value_t high, bool exclusive)
	{
		Range *result = Collector::allocate<Range>(low, high, exclusive);

		return result;
	}
	
	void Range::initialize()
	{
		context->range_class = define_class("Range", context->object_class);
		
		method<Arg::SelfClass<Range>>(context->range_class, "to_s", &to_s);
	}
};

