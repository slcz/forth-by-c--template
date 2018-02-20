#include <iostream>
#include "forth.h"

int main()
{
	using dat = nil;
	using rtn =
	      return_stack<lit<7>,
	      return_stack<lit<3>,
	      return_stack<drop,
	      return_stack<dup, nil>>>>;
	using env = environment<dat, rtn>;
	forth::run<env>::dstack::dump_stack();
}
