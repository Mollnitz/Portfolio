/**
 * Solution to river crossing puzzle with a goat, a cabbage and a wolf.
 * Author: Marius Mikucionis <marius@cs.aau.dk>
 * Compile and run:
 * g++ -std=c++17 -pedantic -Wall -DNDEBUG -O3 -o crossing crossing.cpp && ./crossing
 */
#include "reachability.hpp" // your header-only library solution

#include <functional> // std::function
#include <list>
#include <array>
#include <iostream>

enum actor_t { cabbage, goat, wolf }; // names of the actors
enum class pos_t { shore1, travel, shore2}; // names of the actor positions
using actors_t = std::array<pos_t,3>; // positions of the actors

auto transitions(const actors_t& actors) {
	auto res = std::list<std::function<void(actors_t&)>>{};
	for (auto i=0u; i<actors.size(); ++i)
		switch(actors[i]) {
		case pos_t::shore1:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::travel; });
			break;
		case pos_t::travel:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::shore1; });
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::shore2; });
			break;
		case pos_t::shore2:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::travel; });
			break;
		}
	return res;
}

bool is_valid(const actors_t& actors) {
	// only one passenger:
	if (std::count(std::begin(actors), std::end(actors), pos_t::travel)>1)
		return false;
	// goat cannot be left alone with wolf, as wolf will eat the goat:
	if (actors[actor_t::goat]==actors[actor_t::wolf] && actors[actor_t::cabbage]==pos_t::travel)
		return false;
	// goat cannot be left alone with cabbage, as goat will eat the cabbage:
	if (actors[actor_t::goat]==actors[actor_t::cabbage] && actors[actor_t::wolf]==pos_t::travel)
		return false;
	return true;
}

std::ostream& operator<<(std::ostream& os, const pos_t& pos) {
	switch(pos) {
	case pos_t::shore1: os << "1"; break;
	case pos_t::travel: os << "~"; break;
	case pos_t::shore2: os << "2"; break;
	default: os << "?"; break; // something went terribly wrong
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const actors_t& actors) {
	return os << actors[actor_t::cabbage]
			  << actors[actor_t::goat]
			  << actors[actor_t::wolf];
}

std::ostream& operator<<(std::ostream& os, std::list<const actors_t*>& trace) {
	auto step = 0u;
	for (auto* actors: trace)
		os << step++ << ": " << *actors << '\n';
	return os;
}

void solve(){
	auto state_space = state_space_t(
		actors_t{},                        // initial state
		successors<actors_t>(transitions), // successor generator
		&is_valid);                        // invariant over all states
	auto solution = state_space.check(
		[](const actors_t& actors){ // all actors should be on the shore2:
			return std::count(std::begin(actors), std::end(actors), pos_t::shore2)==actors.size();
		});
	for (auto&& trace: solution)
		std::cout << "#  CGW\n" << trace;
}

int main(){
	solve();
}

/** Sample output:
#  CGW
0: 111
1: 1~1
2: 121
3: ~21
4: 221
5: 2~1
6: 211
7: 21~
8: 212
9: 2~2
10: 222
*/
