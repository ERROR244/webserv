#include <map>

using namespace std;

class A {
	int aa;
	int bb;
	public:
		A(int aa, int bb) : aa(aa), bb(bb) {}
};

int main() {
	map<int, A> m_tmp;
	
	m_tmp.insert(make_pair(1, A(1, 2)));
}