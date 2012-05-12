#include "file.hpp"
#include "../runtime.hpp"
#include "../platform/platform.hpp"
#include "string.hpp"
#include "fixnum.hpp"

namespace Mirb
{
	CharArray File::normalize_path(const CharArray &path)
	{
		CharArray result(path);

		result.localize();

		for(size_t i = 0; i < result.size(); ++i)
			if(result[i] == '\\')
				result[i] = '/';

		if(result[result.size()-1] == '/')
			result.shrink(result.size() - 1);

		return result;
	}
	
	bool File::absolute_path(const CharArray &path)
	{
#ifdef WIN32
		return path.size() > 1 && path[1] == ':';
#else
		return path.size() > 0 && path[0] == '/';
#endif
	}

	void JoinSegments::push(const CharArray &path)
	{
		File::normalize_path(path).split([&](const CharArray &part) {
			segments.push_back(part);
		}, CharArray("/"));
	}

	CharArray JoinSegments::join()
	{
		int pos = 0;

		CharArray result;

		for(size_t i = segments.size(); i-- > 0;)
		{
			if(segments[i] == ".")
				continue;
				
			if(segments[i] == "..")
			{
				pos--;
				continue;
			}

			if(pos < 0)
				pos++;
			else
				result = segments[i] + "/" + result;
		}

		if(result.size())
			result.shrink(result.size() - 1);

		return result;
	}
	
	CharArray File::expand_path(CharArray relative)
	{
		if(!absolute_path(relative))
		{
			JoinSegments joiner;
			joiner.push(Platform::cwd());
			joiner.push(relative);

			return joiner.join();
		}
		else
			return relative;
	};
	
	CharArray File::expand_path(CharArray relative, CharArray from)
	{
		if(!absolute_path(relative))
		{
			JoinSegments joiner;

			if(!absolute_path(from))
				from = expand_path(from);

			joiner.push(from);
			joiner.push(relative);

			return joiner.join();
		}
		else
			return relative;
	};
	
	value_t File::rb_expand_path(String *relative, String *absolute)
	{
		if(absolute)
			return File::expand_path(relative->string, absolute->string).to_string();
		else
			return File::expand_path(relative->string).to_string();
	}
	
	CharArray File::basename(CharArray path)
	{
		CharArray result = normalize_path(path);

		size_t i = result.size();

		while(i-- > 0)
			if(const_cast<const CharArray &>(result)[i] == '/')
				break;
		
		return result.copy(++i, result.size());
	}

	value_t dirname(String *path)
	{
		CharArray result = File::normalize_path(path->string);

		for(size_t i = result.size(); i-- > 0;)
			if(result[i] == '/')
			{
				if(i)
				{
					result.shrink(i);
					return result.to_string();
				}
				else
					return CharArray("/").to_string();
			}
		
		return CharArray(".").to_string();
	}
	
	value_t join(size_t argc, value_t argv[])
	{
		JoinSegments joiner;

		for(size_t i = 0; i < argc; ++i)
		{
			if(type_error(argv[i], context->string_class))
				return 0;

			joiner.push(cast<String>(argv[i])->string);
		}

		return joiner.join().to_string();
	}
	
	void File::initialize()
	{
		context->file_class = define_class("File", context->io_class);
		
		singleton_method<Arg::Class<String>, Arg::DefaultClass<String>>(context->file_class, "expand_path", &rb_expand_path);
		singleton_method<Arg::Class<String>>(context->file_class, "dirname", &dirname);
		singleton_method<Arg::Count, Arg::Values>(context->file_class, "join", &join);

		set_const(context->file_class, Symbol::get("SEPARATOR"), String::from_literal("/"));

#ifdef WIN32
		set_const(context->file_class, Symbol::get("FNM_SYSCASE"), Fixnum::from_int(8));
		set_const(context->file_class, Symbol::get("ALT_SEPARATOR"), String::from_literal("\\"));
#else
		set_const(context->file_class, Symbol::get("FNM_SYSCASE"), Fixnum::from_int(0));
		set_const(context->file_class, Symbol::get("ALT_SEPARATOR"), value_nil);
#endif
	}
};

