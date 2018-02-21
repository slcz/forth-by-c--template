#include <iostream>
#include "forth.h"

int main()
{
	names(dup2);
	/*
	using rtnstk = build_return_stack<
		colon<dup2, dup, dup>,
		num<7>,
		num<3>,
		num<8>,
		num<10>,
		invert,
		num<2>,
		dict
		>::run;
	*/
	using rtnstk = build_return_stack<
		colon<dup2, dup, dup>,
		num<3>,
		lit<dup2>,
		fromr,
		dup,
		dup
		>::run;
	using env = environment<nil, rtnstk, initialize_dictionary::run>;
	forth::run<env>::dstack::tochars();
	std::cout << std::endl;
}
