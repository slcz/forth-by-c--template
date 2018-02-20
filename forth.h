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

template <typename...> struct colon;

template <typename Word, typename... Others>
struct colon<Word, Others...> {
	using others = colon<Others...>;
	template <typename Environment>
		using run = typename others::template action<Environment>;
	template <typename Environment>
		using action = typename others::template
		action<environment<typename Environment::dstack,
		       return_stack<Word, typename Environment::rstack>>>;
};

template<> struct colon<> {
	template <typename Environment> using run = Environment;
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
