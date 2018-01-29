#include <string>
#include "Sierra/SubParsers/type_parser.h"

using namespace archivist::sierra;
using namespace parsers;

SierraField archivist::sierra::parsers::parse_field(SierraContext& ctx, parsing_context& parsectx)
{
	SierraToken offset,type,name,defaultval;
	SierraField ret;
	try{
		offset=parsectx.next();
		if(offset.type!=tk_type::TK_integer)
		{
			throw std::runtime_error(
				std::string("Expected integer, got")
				+std::string(":")
				+offset.token
			);
		}
		type=parsectx.next();
		auto it_type = ctx._types.begin();
		if(type.type==tk_type::TK_symbol)
		{
			it_type = ctx._types.find(type.token);
			if(it_type==ctx._types.end())
			{
				throw std::runtime_error(
					std::string("undefined type")
					+std::string(":")
					+type.token
				);
			}
		}else if(type.type==tk_type::TK_keyword && type.token=="bin"){
			auto open=parsectx.next();
			if(open.type!=tk_type::TK_limit || open.token!="(")
			{
				throw std::runtime_error(
					std::string("Expected '(', got")
					+std::string(":")
					+open.token
				);
			}

			auto field_size=parsectx.next();
			if(field_size.type!=tk_type::TK_integer)
			{
				throw std::runtime_error(
					std::string("Expected integer, got")
					+std::string(":")
					+field_size.token
				);
			}

			auto close=parsectx.next();
			if(close.type!=tk_type::TK_limit || close.token!=")")
			{
				throw std::runtime_error(
					std::string("Expected ')', got")
					+std::string(":")
					+close.token
				);
			}

			std::string binfield_name=std::string("bin(")+field_size.token+")";
			it_type = ctx._types.find(binfield_name);
			if(it_type==ctx._types.end())
			{
				char* dummy=(char*)field_size.token.c_str()+field_size.token.size();
				SierraType sierra_hwuint8;
				sierra_hwuint8.size=strtoll(field_size.token.c_str(),&dummy,10);;
				sierra_hwuint8.name=binfield_name;
				ctx._types[sierra_hwuint8.name]=sierra_hwuint8;
				SierraField single;
				single.name="self";
				single.converter=[=](char* in)->char* {
					char* ptr=(char*)sierra_allocate(sierra_hwuint8.size);
					memcpy(ptr,in,sierra_hwuint8.size);
					return ptr;
				};
				single.offset=0;
				single.type=&ctx._types[sierra_hwuint8.name];
				ctx._types[sierra_hwuint8.name].fields.push_back(single);
			}
			it_type = ctx._types.find(binfield_name);

		}else{
			throw std::runtime_error(
				std::string("Expected symbol, got")
				+std::string(":")
				+type.token
			);
		}
		name=parsectx.next();
		if(name.type!=tk_type::TK_symbol)
		{
			throw std::runtime_error(
				std::string("Expected symbol, got")
				+std::string(":")
				+name.token
			);
		}

		auto tmp=parsectx.next();
		if(tmp.type!=tk_type::TK_statement_end || tmp.token!=";")
		{
			throw std::runtime_error(
				std::string("Expected ';', got")
				+std::string(":")
				+tmp.token
			);
		}

		ret.name=name.token;
		ret.type=&(*it_type).second;
		char* dummy=(char*)offset.token.c_str()+offset.token.size();
		ret.offset=strtoll(offset.token.c_str(),&dummy,10);
	}catch(token_expected_exception& e)
	{
		parsectx.index--;
		throw std::runtime_error(
			std::string("Met EOF during declaration")
		);
	}
	return ret;
}

void archivist::sierra::parsers::parse_type(SierraContext& ctx, parsing_context& parsectx)
{
	SierraToken name,tmp;
	std::vector<SierraField> fields;
	try{
		name=parsectx.next();
		if(name.type!=tk_type::TK_symbol)
		{
			throw std::runtime_error(
				std::string("Expected symbol name, got")
				+std::string(":")
				+name.token
			);
		}
	}catch(token_expected_exception& e)
	{
		parsectx.index--;
		throw std::runtime_error(
			std::string("Expected symbol name but none was provided")
		);
	}
	try{
		tmp=parsectx.next();
		if(tmp.type!=tk_type::TK_limit || tmp.token!="{")
		{
			throw std::runtime_error(
				std::string("Expected '{' name, got")
				+std::string(":")
				+tmp.token
			);
		}
	}catch(token_expected_exception& e)
	{
		parsectx.index--;
		throw std::runtime_error(
			std::string("Met EOF while expecting '{'")
		);
	}

	while(tmp.type!=tk_type::TK_limit || tmp.token!="}")
	{
		fields.push_back(archivist::sierra::parsers::parse_field(ctx,parsectx));
		
		try{
			tmp=parsectx.next();
			parsectx.index--;
		}catch(token_expected_exception& e)
		{
			throw std::runtime_error(
				std::string("Met EOF while expecting a valid field or a '}'")
			);
		}
	}
	parsectx.index+=2;
	size_t max_offset=0;
	for(auto n : fields)
		max_offset=std::max(n.offset+n.type->size,max_offset);
	SierraType type = {name.token,fields,max_offset};
	ctx._types[type.name]=type;
}