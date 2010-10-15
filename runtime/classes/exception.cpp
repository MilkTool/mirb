#include "../classes.hpp"
#include "../runtime.hpp"
#include "exception.hpp"
#include "string.hpp"
#include "symbol.hpp"

rt_value rt_Exception;

/*
 * Exception
 */

rt_compiled_block(rt_Exception_allocate)
{
	rt_value exception = rt_alloc(sizeof(struct rt_exception));

	RT_COMMON(exception)->flags = C_EXCEPTION;
	RT_COMMON(exception)->class_of = rt_Exception;
	RT_COMMON(exception)->vars = 0;

	RT_EXCEPTION(exception)->message = RT_NIL;
	RT_EXCEPTION(exception)->backtrace = RT_NIL;

	return exception;
}

rt_compiled_block(rt_exception_to_s)
{
	return RT_EXCEPTION(obj)->message;
}

rt_compiled_block(rt_exception_initialize)
{
	RT_EXCEPTION(obj)->message = RT_ARG(0);

	return obj;
}

void rt_exception_init(void)
{
	rt_Exception = rt_define_class(rt_Object, "Exception", rt_Object);

	rt_define_singleton_method(rt_Exception, "allocate", rt_Exception_allocate);

	rt_define_method(rt_Exception, "initialize", rt_exception_initialize);
	rt_define_method(rt_Exception, "message", rt_exception_to_s);
	rt_define_method(rt_Exception, "to_s", rt_exception_to_s);
}
