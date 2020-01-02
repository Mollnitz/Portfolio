#ifndef REACHABILITY

#define REACHABILITY

#define REACHABILITY_DEBUG false

#include <ostream>
#include <list>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <memory> // This is memory_resource in windows.
#include <iterator>
#include <initializer_list>
#include <set>
#include <queue>
#include <type_traits>
#include <algorithm>

using namespace std;

//Function was missing in code provided, so I made my own. - It's kinda lame because it only takes a string.
/*Paints out a string, ends the line*/
void log( string input) {
#if REACHABILITY_DEBUG
    cout << input << endl;
#endif // REACHBILITYDEBUG
};


/* Part of: 5. Support various search orders: breadth-first search, depth-first search (the leaping frogs have different solutions). */
/*Details the order, depth_first or breadth_first, notice that this library uses breadth_first as its default order */
enum class search_order_t
{
	depth_first,
	breadth_first
};

/*
Performs a search with a priority queue.
state: The initial state.
cost: The initial cost.
cost_generator: A function that generates the next cost, using the previous cost, and previous state.
successor_generator: A function that generates all possible successor states to the current state.
valid_state_func: A function that checks if a state is a valid state.
accept_state_func A function that checks if a state is an acceptable state.
comparison_func: the comparison function used by the priority queue itself, bfs or other costs = std::greater && dfs = std::less
*/
template <class State, class Cost>
list<list<shared_ptr<State>>> priority_queue_search(State state, Cost cost, function<vector<State>(State&)> successor_generator, function<Cost(State&, Cost&)> cost_generator, function<bool(const State&)> valid_state_func, function<bool(const State&)> accept_state_func, function<bool(const pair<Cost, State>&, const pair<Cost, State>&)> comparison_func)
{
	using pair_type = pair<Cost, State>;
	using comp_type = function<bool(const pair_type&, const pair_type&)>;
	
	// a set that holds all seen states up to this point.
	set<State> seen_set{};

	auto result = list<list<shared_ptr<State>>>{};
	unordered_map<State, State> child_to_parent_map = unordered_map<State, State>{};

	//Priority maps are sorted by the first element in a pair structure, that is why it's Pair<cost,state>. 
	auto pqs_queue = priority_queue<pair_type, vector<pair_type>, comp_type> {comparison_func};
	//Emplace smartly picks constructor.
	pqs_queue.emplace(cost, state);

	//Infinite loop protection
	seen_set.insert(state);

	while (!pqs_queue.empty())
	{
		pair_type parent_item = pqs_queue.top();
		pqs_queue.pop();

		if (accept_state_func(parent_item.second))
		{
			result.push_back(map_ancestry_shared(child_to_parent_map, parent_item.second));
			log("Added a result");
		}
		else if (valid_state_func(parent_item.second)) {
			auto successors = successor_generator(parent_item.second);
			for (auto&& child_item : successors)
			{
				//Keeps track of seen items, avoid infinite loops
				if (seen_set.count(child_item) == 0) //Set is faster than vector (lg(n))
				{
					seen_set.insert(child_item);
					child_to_parent_map[child_item] = parent_item.second;
					pqs_queue.push(make_pair(cost_generator(parent_item.second, parent_item.first), child_item));
				}
			}
		}
	}
	return result; //RVO - ITEM 25
}

/*Gets a route from the acceptable state child (or any child really) though the chain of parents, to the root node*/
template <class State>
list<shared_ptr<State>> map_ancestry_shared(unordered_map<State, State> c_to_p_map, State child) {
	auto ret = list<shared_ptr<State>>{};
	ret.push_back(make_shared<State>(child));
	// https://en.cppreference.com/w/cpp/container/unordered_map/find - if it doesn't find the element, it returns end iterator.
	auto c_to_p_pair = c_to_p_map.find(child);
	while (c_to_p_pair != c_to_p_map.end())
	{
		ret.push_back(make_shared<State>(c_to_p_pair->second));
		c_to_p_pair = c_to_p_map.find(c_to_p_pair->second);
	}
	return ret;
}




//9. The implementation should be generic and applicable to all puzzles using the same library templates. If the search order or cost are not specified, the library should use reasonable defaults.
/*Mimics `cost_t` struct from family.cpp, and lets this facimily make BFS and DFS search for any type*/

/*Default cost structure, simply annotates costs in the conventional BFS / DFS manner, as nodes are discovered, chronologically*/
struct default_cost_t {
	int id;
	static int total;
	default_cost_t()
	{
		id = total++;
	}
	
	bool operator<(const default_cost_t& other) const {
		return id < other.id;
	}
};

//9. The implementation should be generic and applicable to all puzzles using the same library templates. If the search order or cost are not specified, the library should use reasonable defaults.
/*Cheap cost_gen, that merely annotates costs in the conventional BFS and DFS manner, chronologically*/
template <class State, class default_cost_t>
default_cost_t cost_gen(const State& prev_state, const default_cost_t& prev_cost) {
	return default_cost_t{}; //Use default constructor.
}

int default_cost_t::total = 0;


//9. The implementation should be generic and applicable to all puzzles using the same library templates. If the search order or cost are not specified, the library should use reasonable defaults.
/*Cheap way to ensure all states are valid*/
template <class State>
bool return_true(const State& state) {
	return true;
}


template <class State, class Cost = default_cost_t, class CostFn = function<Cost(const State&, const Cost&)>>
class state_space_t
{
public:
	
	state_space_t() = delete; //No default constructor - library should not be used like this.
	~state_space_t() = default; //Default deleter - rule of zero.


	/* 3. Find a state satisfying the goal predicate when given the initial state and successor generating function. */
	state_space_t(State state, function<vector<State>(State&)> successor_generator) : state_space_t(state, successor_generator, return_true<State>) { } ;

	/* 6. Support a given invariant predicate (state validation function, like in river crossing puzzles). */
	/* 9. The implementation should be generic and applicable to all puzzles using the same library templates. If the search order or cost are not specified, the library should use reasonable defaults. */
	state_space_t(State state, function<vector<State>(State&)> successor_generator, bool (*val_gen) (const State&)) : state_space_t(state, Cost{}, successor_generator, val_gen, cost_gen<State, Cost>) {
		default_cost_t::total = 0; // reset node index inbetween runs. - only needed for this constructor, and the one above.
	};
	
	/* 7. Support custom cost function over states (like noise in Japanese river crossing puzzle).  */
	state_space_t(State state, Cost init_cost,  function<vector<State>(State&)> succ_gen, bool(*val_gen) (const State&), CostFn cost_succ_gen) : cost_generator(cost_succ_gen) {
		//Generic type static asserts.
		
		/* 10. User friendly to use and fail with a message if the user supplies arguments of wrong types. */
		static_assert(is_class<State>::value, "Template argument state must be struct (or class)");
		static_assert(is_class<Cost>::value, "Template argument init_cost must be struct (or class)");
		static_assert(is_convertible<CostFn, function<Cost (const State&, const Cost&)>>::value, "Template argument cost_succ_gen must be function or lambda, that is convertible to  function<U (const T&, const U&)>>");

		//Assignments - note the cost_generator assignment is above to avoid constructing a lambda.
		this->first_state = state;
		this->successor_generator = succ_gen;
		this->valid_state_func = val_gen;
		this->first_cost = init_cost;
	};

    //Check with sensible default = breadth first search.
    //http://mishadoff.com/images/dfs/binary_tree_search.png - visual aid
    /* 5. Support various search orders: breadth-first search, depth-first search (the leaping frogs have different solutions) */
    auto check(function<bool(const State&)>  accept_func, search_order_t order = search_order_t::breadth_first) {
        list<list<shared_ptr<State>>> search; //Result
        if (order == search_order_t::breadth_first)
        {
            search = priority_queue_search<State, Cost>(first_state, first_cost, successor_generator, cost_generator, valid_state_func, accept_func, greater<pair<Cost, State>>());
        }
        else
        {
            search = priority_queue_search<State, Cost>(first_state, first_cost, successor_generator, cost_generator, valid_state_func, accept_func, less<pair<Cost, State>>());
        }
        //No need to use move here due to RVO
        return search;
    }

private:

	State first_state;
	Cost first_cost;

	//This could say ' function<Cost (const State&, const Cost&)>' as type.
	CostFn cost_generator;

	function<vector<State>(State&)> successor_generator;
	function<bool(const State&)> valid_state_func;

};


//1. Extend  hash function for an arbitrary (iterable) container of basic types.
namespace std {

    /* FULL DISCLOSURE, THIS WAS WRITTEN BY Marius Mikucionis http://people.cs.aau.dk/~marius/Teaching/AP2019/lecture9.html#/7/4 */
    template <typename... Ts>
    using void_t = void; // eats all valid types
    template <typename T, typename = void> // primary declaration
    struct is_container: std::false_type {};  // computes false
    template <typename C>  // specialization
    struct is_container<C, // type c examination
            void_t< // the following must evaluate to types:
                    //typename C::iterator,          // check subtype
                    //typename C::const_iterator,    // check subtype
                    //is_array<C>,
                    decltype(std::begin(std::declval<C&>())), // check iteration:
                    decltype(std::end(std::declval<C&>()))    // create C() and call
            > // finished checks
    > : std::true_type {}; // computes "true"
    template <typename C>  // *_t type alias
    using is_container_t = typename is_container<C>::type;
    template <typename C>  // *_v value
    constexpr auto is_container_v = is_container<C>::value;

    template<template<typename, typename...> class Cont, typename T, typename... N>
    struct hash<Cont<T, N...>> {
        enable_if_t<is_container_v<Cont<T, N...>>, size_t>  //enable_if_t<is_container_v<Cont<T, N...>> || is_array<Cont<T, N...>>::value, size_t>
        operator()(const Cont<T, N...> &container) const {

            hash<T> hashT;
            auto res = 0;
            for (auto&& val : container)
            {
                res = (res << 1) ^ hashT(val);
            }
            return res;
        }
    };

	template<class T, size_t U>
	struct hash<array<T,U>> {
        auto operator() (const array<T,U>& data) const {
			hash<T> hashT;
			size_t res = 0;
			for (auto&& val : data) // I love these loops
			{
				res = (res << 1) ^ hashT(val);
			}
			return res;
		}
	};

    /* // Use this to compile with Clang / MSVS
	template<typename T>
	struct hash <vector<T>> {
		auto operator() (const vector<T>& data) const {
			hash<T> hashT;
			size_t res = 0;
			for (auto&& val : data)
			{
				res = (res << 1) ^ hashT(val);
			}
			return res;
		}
	}; */
};


/* 4. Print the trace of a state sequence from the initial state to the found goal state. */
template <class State>
ostream& operator <<(ostream& os, const list<shared_ptr<State>>& lst)
{
	int i = 0;
	//Print backwards - avoids having to shuffle list.
	for (auto end = lst.rbegin(), front = lst.rend(); end != front; end++, i++)
	{
		os << i << " : " << **end <<  endl;
	} 
#if REACHABILITY_DEBUG
	cin >> i; //Pause here because it's nice to see results - a real library obviously shouldn't do this.
#endif // REACHABILITY_DEBUG
return os;
}

/*
2. Create a generic successor generator function out of a transition generator function.
	A transition generator function generates functions that change a state.
	Each such function corresponds to a transition.
	A successor generator function gets a state and generates a set of its successor states.
*/
//Would have used auto here, but it narrowed it out and gave me 60 compiler errors, no fun.

/*Turns a transition generator function (transformer) into a successor generator function*/
template <class State>
function<vector<State>(State&)> successors(function<list<function<void(State&)>>(const State&)> transformer)
{
	return [transformer](const State& data){
		vector<State> res{};
		for (auto&& func: transformer(data)) //For each function in the resulting list.
		{
			State copy = data;
			func(copy);
			res.push_back(copy);
		}
		return res;
	};
}
#endif // !REACHABILITY