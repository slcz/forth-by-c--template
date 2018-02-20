#include <iostream>
#include "forth.h"

int main()
{
	using dup2 = colon<dup, dup>;
	using rtnstk = build_return_stack<lit<7>, lit<3>, drop, dup, dup2>::run;
	using env = environment<nil, rtnstk>;
	forth::run<env>::dstack::dump_stack();
}
