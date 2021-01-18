#include "Bitvector.h"
#include "Parser.h"
#include "Matcher.h"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

signed main(int argc, char** argv){
	std::cin.tie(nullptr);
	std::ios::sync_with_stdio(false);
	if(argc != 3){
		std::cerr << "arg: patternfile textfile" << std::endl;
		return 1;
	}
	std::string pattern;
	std::ifstream ifs(argv[1]);
	if(!ifs){
		std::cerr << "unable to open " << argv[1] << " for pattern" << std::endl;
		return 1;
	}
	std::getline(ifs, pattern);
	auto start = std::chrono::system_clock::now();
	Parser parser(pattern);
	Matcher matcher(parser.get_tnfa());
	//std::cerr << "state_size: " << matcher.state_size() << std::endl;
	//std::cerr << "depth: " << matcher.depth() << std::endl;
	std::ifstream textfile(argv[2]);
	if(!textfile){
		std::string str = argv[2];
		std::cerr << "unable to open " << std::string(argv[2]) << " for textfile" << std::endl;
		return 1;
	}
	auto preprocess = std::chrono::system_clock::now();
	for(std::string text; getline(textfile, text);) {
		auto midx = matcher.match(text);
		for(auto e : midx) {
			std::cout << e << " ";
		}
		if(midx.size() == 0) std::cout << -1;
		std::cout << '\n';
	}
	auto end = std::chrono::system_clock::now();
	auto preprocess_time = std::chrono::duration_cast<std::chrono::microseconds>(preprocess - start);
	auto running_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - preprocess);
	std::cerr << argv[0] << ',' << argv[1] << ',' << argv[2] << ',';
	std::cerr << preprocess_time.count() << ',';
	std::cerr << running_time.count();

	return 0;
}

