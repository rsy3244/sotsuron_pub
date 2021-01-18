#include <bits/stdc++.h>

#define ALL(a) (a).begin(), (a).end()

using namespace std;

signed main(int argc, char** argv){
	if(argc != 2) {
		cout << "args: class" << endl;
		return 1;
	}

	string infilename("pattern/"s + argv[1] + "/");
	ifstream ifs(infilename + "32.txt");
	if(!ifs){
		cout << "file error" << endl;
		return 1;
	}
	string patternbase;
	if(!getline(ifs, patternbase)){
		cerr << "cannot read: " << infilename + "/32.txt" << endl;
	}

	string output(patternbase);
	for(int i = 64; i < 1024; i+= 32) {
		output += ("|" + patternbase);
		cerr << "output: " + infilename + to_string(i) + ".txt" << endl;
		ofstream ofs(infilename + to_string(i) + ".txt");
		ofs << output << endl;
	}

	return 0;
}

