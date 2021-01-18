#include <bits/stdc++.h>
#include <functional>
#include <optional>

using namespace std;

optional<int> getid(const vector<int> &a, size_t idx) {
	if( idx < a.size()) return a.at(idx);
	return std::nullopt;
}

signed main(){
	vector<int> arr{0,1,2,3,4};
	for(size_t i = 0; i < 7; i++){
		if(auto id = getid(arr, i)){
			cout << *id << endl;
		} else{
			cout << -1 << endl;
		}
	}


	return 0;
}

