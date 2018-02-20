#pragma once

struct nil {
	static void dump_stack() { std::cout << std::endl; }
};

template <typename Element, typename Tail>
struct return_stack {
	using head = Element;
	using tail = Tail;
};

template <int Element, typename Tail>
struct data_stack {
	static const int head = Element;
	using tail = Tail;
	static void dump_stack() {
		std::cout << head << " ";
		tail::dump_stack();
	}
};

template <typename DataStack, typename ReturnStack>
struct environment {
	using dstack = DataStack;
	using rstack = ReturnStack;
};

/* predefined word */
template <typename, typename...> struct build_return_stack;
template <typename Head>
struct build_return_stack<Head> {
	using run = return_stack<Head, nil>;
};

template <typename Head, typename... Rest>
struct build_return_stack {
	using run = return_stack<Head,
	      typename build_return_stack<Rest...>::run>;
};

template <typename, typename> struct rstack_concat;

template <typename S1, typename S2>
struct rstack_concat {
	using run = return_stack<typename S1::head,
	      typename rstack_concat<typename S1::tail, S2>::run>;
};

template <typename S2>
struct rstack_concat<nil, S2> {
	using run = S2;
};

template <typename... Words>
struct colon {
	using create = typename build_return_stack<Words...>::run;
	template <typename E>
		using run = environment<typename E::dstack,
		      typename rstack_concat<create, typename E::rstack>::
			      run>;
};

template <typename DataStack, typename ReturnStack>
struct interpreter {
	using word = typename ReturnStack::head;
	using rest = typename ReturnStack::tail;
	using runone = typename word::template run<environment<DataStack, rest>>;
	using run = typename interpreter<typename runone::dstack,
	                                 typename runone::rstack>::run;
};

template <typename DataStack>
struct interpreter<DataStack, nil> {
	using run = environment<DataStack, nil>;
};

struct forth {
	template <typename Environment>
		using run = typename interpreter<
		typename Environment::dstack,
		typename Environment::rstack>::run;
};

template<int N>
struct lit {
	template <typename Environment>
		using run = environment<data_stack<N,
		      typename Environment::dstack>,
		      typename Environment::rstack>;
};

struct drop {
	template <typename Environment>
		using run = environment<typename Environment::dstack::tail,
		      typename Environment::rstack>;
};

struct dup {
	template <typename Environment>
		using run = environment<data_stack<Environment::dstack::head,
		      typename Environment::dstack>,
		      typename Environment::rstack>;
};
