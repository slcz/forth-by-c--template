#pragma once

#include "map.h"

/*#define name(x) struct x {}*/
#define genstruct(x) struct x {};
#define names(...) MAP(genstruct, __VA_ARGS__)

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

template <typename DataStack, typename ReturnStack, typename Dictionary>
struct environment {
	using dstack = DataStack;
	using rstack = ReturnStack;
	using dict   = Dictionary;
};

/* predefined word */
template <typename S1, typename S2>
struct rstack_concat {
	using run = return_stack<typename S1::head,
	      typename rstack_concat<typename S1::tail, S2>::run>;
};

template <typename S2>
struct rstack_concat<nil, S2> {
	using run = S2;
};

template <typename, typename...> struct build_return_stack;

struct literal;
template <int> struct L;
template <int N>
struct lit {
	using run = return_stack<literal, return_stack<L<N>, nil>>;
};

struct def;
template <typename, typename...> struct wordlist;
template <typename... List>
struct colon {
	using run = return_stack<def, return_stack<wordlist<List...>, nil>>;
};

template <template<typename> typename Head, typename... List>
struct build_return_stack<Head<List...>> {
	using head_expansion = typename Head<List...>::run;
	using run = typename rstack_concat<head_expansion, nil>::run;
};

template <template<typename...> typename Head, typename... List,
	 typename... Rest>
struct build_return_stack<Head<List...>, Rest...> {
	using head_expansion = typename Head<List...>::run;
	using run = typename rstack_concat<head_expansion,
	      typename build_return_stack<Rest...>::run>::run;
};

template <template<int> typename Head, int N>
struct build_return_stack<Head<N>> {
	using head_expansion = typename Head<N>::run;
	using run = typename rstack_concat<head_expansion, nil>::run;
};

template <template<int> typename Head, int N, typename... Rest>
struct build_return_stack<Head<N>, Rest...> {
	using head_expansion = typename Head<N>::run;
	using run = typename rstack_concat<head_expansion,
	      typename build_return_stack<Rest...>::run>::run;
};

template <typename Head>
struct build_return_stack<Head> {
	using run = return_stack<Head, nil>;
};

template <typename Head, typename... Rest>
struct build_return_stack {
	using run = return_stack<Head,
	      typename build_return_stack<Rest...>::run>;
};

template <bool B, typename T, typename F> struct If      { using v = T; };
template <typename T, typename F> struct If<false, T, F> { using v = F; };

template <typename T, typename U> struct Eq { const static bool v = false; };
template <typename T> struct Eq<T,T> { const static bool v = true;  };

template <typename Key, typename Value, typename Rest>
struct dictionary {
	using key   = Key;
	using value = Value;
	using rest  = Rest;
};

struct lookup {
	template <typename Key, typename Dict>
	struct search {
		using v = typename
			If <
			Eq<typename Dict::key, Key>::v,
			typename Dict::value,
			typename search<Key, typename Dict::rest>::v>::v;
	};
	// insert a default word when the key is not defined?
	template <typename Key>
	struct search<Key, nil> { using v = nil; };
	template <typename Key, typename Dict>
		using run = typename search<Key, Dict>::v;
};

template <typename words>
struct R {
	template <typename E>
		using run = environment<
			typename E::dstack,
			typename rstack_concat<words, typename E::rstack>::run,
			typename E::dict>;
};

template <typename Key, typename... Words>
struct wordlist {
	template<typename Dict>
		using v = dictionary<
			Key,
			R<typename build_return_stack<Words...>::run>,
			Dict>;
};

struct def {
	template <typename E>
	using newdict = typename E::rstack::head::template v<typename E::dict>;
	template <typename E>
		using run = environment<
				typename E::dstack,
				typename E::rstack::tail,
				newdict<E>>;
};

template <typename DataStack, typename ReturnStack, typename Dict>
struct interpreter {
	using first  = lookup::run<typename ReturnStack::head, Dict>;
	using rest   = typename ReturnStack::tail;
	using runone = typename first::template
		run<environment<DataStack, rest, Dict>>;
	using run = typename interpreter<typename runone::dstack,
	                                 typename runone::rstack,
					 typename runone::dict>  ::run;
};

template <typename DataStack, typename Dict>
struct interpreter<DataStack, nil, Dict> {
	using run = environment<DataStack, nil, Dict>;
};

struct forth {
	template <typename Environment>
		using run = typename interpreter<
		typename Environment::dstack,
		typename Environment::rstack,
		typename Environment::dict   >::run;
};

template <int N> struct L { static const int v = N; };

struct literal {
	template <typename Environment>
		static const int N = Environment::rstack::head::v;
	template <typename Environment>
		using run = environment<data_stack<N<Environment>,
		      typename Environment::dstack>,
		      typename Environment::rstack::tail,
		      typename Environment::dict>;
};

/* Builtin words */
struct drop {
	template <typename Environment>
		using run = environment<
		      typename Environment::dstack::tail,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct swap {
	template <typename Stack> struct do_swap {
			const static int a0 = Stack::head;
			const static int a1 = Stack::tail::head;
			using run = data_stack<
			    a1, data_stack<a0, typename Stack::tail::tail>>;
		};
	template <typename Environment> using run = environment<
		      typename do_swap<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct dup {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head,
		                 typename Environment::dstack>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct over {
	template <typename Stack> struct do_over {
			const static int a0 = Stack::head;
			const static int a1 = Stack::tail::head;
			const static int a2 = Stack::tail::tail::head;
			using run = data_stack<a2, data_stack<a1,
			    data_stack<a0, typename Stack::tail::tail::tail>>>;
		};
	template <typename Environment> using run = environment<
		      typename do_over<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct rot {
	template <typename Stack> struct do_rot {
			const static int a0 = Stack::head;
			const static int a1 = Stack::tail::head;
			const static int a2 = Stack::tail::tail::head;
			using run = data_stack<a2, data_stack<a0,
			    data_stack<a1, typename Stack::tail::tail::tail>>>;
		};
	template <typename Environment> using run = environment<
		      typename do_rot<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct nrot {
	template <typename Stack> struct do_nrot {
			const static int a0 = Stack::head;
			const static int a1 = Stack::tail::head;
			const static int a2 = Stack::tail::tail::head;
			using run = data_stack<a1, data_stack<a2,
			    data_stack<a0, typename Stack::tail::tail::tail>>>;
		};
	template <typename Environment> using run = environment<
		      typename do_nrot<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct drop2 {
	template <typename Environment>
		using run = environment<
		      typename Environment::dstack::tail::tail,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct swap2 {
	template <typename Stack> struct do_swap2 {
			const static int a0 = Stack::head;
			const static int a1 = Stack::tail::head;
			const static int a2 = Stack::tail::tail::head;
			const static int a3 = Stack::tail::tail::tail::head;
			using run = data_stack<a1, data_stack<a0,
			    data_stack<a3, data_stack<a2,
			    typename Stack::tail::tail>>>>;
		};
	template <typename Environment> using run = environment<
		      typename do_swap2<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct dup2 {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head, data_stack<
		                 Environment::dstack::head,
		                 typename Environment::dstack>>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct ifdup {
	template <typename Stack> struct do_ifdup {
			const static int head = Stack::head;
			using run = typename
				If<head == 0, Stack,
			           data_stack<head, Stack>>::v;
		};
	template <typename Environment> using run = environment<
		      typename do_ifdup<typename Environment::dstack>::run,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct inc {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head + 1,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct dec {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head - 1,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct inc4 {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head + 4,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct dec4 {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head - 4,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct plus {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head +
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct minus {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head -
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct mul {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head *
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct divmod {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head /
		                 Environment::dstack::tail::head,
		                 data_stack<
					 Environment::dstack::head %
					 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct equ {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head ==
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct nequ {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head !=
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct lt {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head <
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct gt {
	template <typename Environment>
		using run = environment<
		      data_stack<(Environment::dstack::head >
		                  Environment::dstack::tail::head),
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct le {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head <=
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct ge {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head >=
		                 Environment::dstack::tail::head,
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct zequ {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head == 0,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct znequ {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head != 0,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct zlt {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head < 0,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct zgt {
	template <typename Environment>
		using run = environment<
		      data_stack<(Environment::dstack::head > 0),
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct zle {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head <= 0,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct zge {
	template <typename Environment>
		using run = environment<
		      data_stack<Environment::dstack::head >= 0,
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct and_ {
	template <typename Environment>
		using run = environment<
		      data_stack<(Environment::dstack::head &
		                  Environment::dstack::tail::head),
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct or_ {
	template <typename Environment>
		using run = environment<
		      data_stack<(Environment::dstack::head |
		                  Environment::dstack::tail::head),
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct xor_ {
	template <typename Environment>
		using run = environment<
		      data_stack<(Environment::dstack::head ^
		                  Environment::dstack::tail::head),
		                 typename Environment::dstack::tail::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct invert {
	template <typename Environment>
		using run = environment<
		      data_stack<(~Environment::dstack::head),
		                 typename Environment::dstack::tail>,
		      typename Environment::rstack,
		      typename Environment::dict>;
};

struct initialize_dictionary {
	using run =                  dictionary<
		drop,     drop,      dictionary<
		dup,      dup,       dictionary<
		literal,  literal,   dictionary<
		swap,     swap,      dictionary<
		over,     over,      dictionary<
		rot,      rot,       dictionary<
		nrot,     nrot,      dictionary<
		drop2,    drop2,     dictionary<
		swap2,    swap2,     dictionary<
		dup2,     dup2,      dictionary<
		ifdup,    ifdup,     dictionary<
		inc,      inc,       dictionary<
		dec,      dec,       dictionary<
		inc4,     inc4,      dictionary<
		dec4,     dec4,      dictionary<
		plus,     plus,      dictionary<
		minus,    minus,     dictionary<
		mul,      mul,       dictionary<
		divmod,   divmod,    dictionary<
		equ,      equ,       dictionary<
		nequ,     nequ,      dictionary<
		lt,       lt,        dictionary<
		gt,       gt,        dictionary<
		le,       le,        dictionary<
		ge,       ge,        dictionary<
		zequ,     zequ,      dictionary<
		znequ,    znequ,     dictionary<
		zlt,      zlt,       dictionary<
		zgt,      zgt,       dictionary<
		zle,      zle,       dictionary<
		zge,      zge,       dictionary<
		zge,      zge,       dictionary<
		and_,     and_,      dictionary<
		or_,      or_,       dictionary<
		xor_,     xor_,      dictionary<
		and_,     and_,      dictionary<
		invert,   invert,    dictionary<
		def,      def,       nil
		>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>;
};
