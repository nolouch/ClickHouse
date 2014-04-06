#pragma once
#include <statdaemons/Exception.h>
#include <zkutil/Types.h>


namespace zkutil
{

class KeeperException : public DB::Exception
{
public:
	KeeperException(const std::string & msg) : DB::Exception(msg), code(ReturnCode::Ok) {}
	KeeperException(const std::string & msg, ReturnCode::type code_)
		: DB::Exception(msg + " (" + ReturnCode::toString(code_) + ")"), code(code_) {}
	KeeperException(ReturnCode::type code_)
		: DB::Exception(ReturnCode::toString(code_)), code(code_) {}

	ReturnCode::type code;
};

};