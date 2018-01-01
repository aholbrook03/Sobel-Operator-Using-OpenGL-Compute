#pragma once

#include <string>
using std::string;

#include <exception>
using std::exception;

class GL43WindowInitException : public exception
{
public:
	GL43WindowInitException(string &error_str) : error_string_(error_str) { }
	GL43WindowInitException(const char *error_str) : error_string_(error_str) { }

	const char * what() const throw()
	{
		return error_string_.c_str();
	}

private:
	string error_string_;
};
