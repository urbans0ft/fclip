#include "LastError.h"
std::wostream& operator<<(std::wostream& os, const LastError& error)
{
	os << error._lastErrorMessage;
	return os;
}