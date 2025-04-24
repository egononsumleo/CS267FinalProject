#include <bits/stdc++.h>
#define rep(i,a,b) for(int i = a; i < b; ++i)
#define all(c) c.begin(), c.end()
#define gmax(x,y) x=max(x,y)
#define gmin(x,y) x=min(x,y)
#define gadd(x,y) x=add(x,y)
#define gmul(x,y) x=mul(x,y)
using namespace std;

typedef pair<int,int> pii;
typedef long long ll;
typedef vector<int> vi;

int main(){
	cin.tie(0);
	cout.tie(0);
	ios_base::sync_with_stdio(0);
	int sz = 1000;
	string s;
	double y = 0;
	while(getline(cin, s)){
		auto ss = stringstream(s);
		double tx,ty;
		ss >> ty >> tx;
		y += ty;
		cout << tx << ' ' << y << '\n';
	}
}

