#include "../classes.h"
#include "../runtime.h"
#include "../constant.h"
#include "../classes/symbol.h"
#include "../classes/string.h"

rt_value rt_Kernel;

/*
 * Kernel
 */

rt_value __stdcall rt_kernel_proc(rt_value obj, rt_value block, size_t argc, rt_value argv[])
{
	if(block)
		return block;
	else
		return RT_NIL;
}

rt_value __stdcall rt_kernel_eval(rt_value obj, rt_value block, size_t argc, rt_value argv[])
{
	rt_value arg = RT_ARG(0);

	if(rt_type(arg) != C_STRING)
		return RT_NIL;

	return rt_eval(obj, rt_string_to_cstr(arg), 0);
}

static FILE *open_file(rt_value *filename)
{
	FILE* file = fopen(rt_string_to_cstr(*filename), "r");

	if(!file)
	{
		rt_value append = rt_dup_string(*filename);

		rt_concat_string(append, rt_string_from_cstr(".rb"));

		file = fopen(rt_string_to_cstr(append), "r");

		if(!file)
			return 0;

		*filename = append;

		return file;
	}

	return file;
}

static rt_value run_file(rt_value self, rt_value *filename)
{
	FILE* file = open_file(filename);

	if(!file)
		return RT_NIL;

	fseek(file, 0, SEEK_END);

	size_t length = ftell(file);

	fseek(file, 0, SEEK_SET);

	char* data = malloc(length + 1);

	if(fread(data, 1, length, file) != length)
	{
		free(data);
		fclose(file);

		return RT_NIL;
	}

	data[length] = 0;

	rt_value result = rt_eval(self, data, rt_string_to_cstr(*filename));

	free(data);
	fclose(file);

	return result;
}

rt_value __stdcall rt_kernel_load(rt_value obj, rt_value block, size_t argc, rt_value argv[])
{
	rt_value filename = RT_ARG(0);

	return run_file(obj, &filename);
}

rt_value __stdcall rt_kernel_print(rt_value obj, rt_value block, size_t argc, rt_value argv[])
{
	RT_ARG_EACH(i)
	{
		if(rt_type(argv[i]) != C_STRING)
			argv[i] = RT_CALL_CSTR(argv[i], "to_s", 0, 0);

		if(rt_type(argv[i]) == C_STRING)
			printf("%s", rt_string_to_cstr(argv[i]));
	}

	return RT_NIL;
}

void rt_kernel_init(void)
{
	rt_Kernel = rt_define_module(rt_Object, rt_symbol_from_cstr("Kernel"));

	rt_include_module(rt_Object, rt_Kernel);

    rt_define_method(rt_Kernel, rt_symbol_from_cstr("proc"), rt_kernel_proc);
    rt_define_method(rt_Kernel, rt_symbol_from_cstr("eval"), rt_kernel_eval);
    rt_define_method(rt_Kernel, rt_symbol_from_cstr("print"), rt_kernel_print);
    rt_define_method(rt_Kernel, rt_symbol_from_cstr("load"), rt_kernel_load);
    rt_define_method(rt_Kernel, rt_symbol_from_cstr("require"), rt_kernel_load);
}
