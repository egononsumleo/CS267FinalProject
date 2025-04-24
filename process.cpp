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
	int sz = 300;
	const int M = 2;
	string s;
	vector<double> a[M];
	int cnt = 0;
	while(getline(cin, s)){
		auto ss = stringstream(s);
		rep(i,0,M){
			double x;
			ss >> x;
			a[i].push_back(x);
		}
		++cnt;
	}

	rep(i,0,a[0].size()-sz+1){
		vi v;
		rep(j,1,M){
			double cnt = 0;
			rep(k,0,sz){
				cnt += a[j][i+k];
			}
			v.push_back(cnt);
		}
		cout << a[0][i];
		for(double x:v){
			cout << ' ' << double(x)/sz;
		}
		cout << '\n';
	}
}
