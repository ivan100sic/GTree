#include "GTree.h"
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
using namespace gtree;
using namespace std;

void testDestructor(int n){
	GTree<int> drvo;
	for (int i = 1; i <= n; i++) drvo.insert(i);
	drvo.erase(n / 2);
	drvo.erase(n / 3);
	drvo.erase(n / 5);
}

GTree<string, int>& population(){
	static GTree<string, int> population;
	static bool alive = false;
	if (!alive){
		population.insert("Russia", 144031000);
		population.insert("Germany", 81276000);
		population.insert("Turkey", 78214000);
		population.insert("France", 67063000);
		population.insert("United Kingdom", 65081276);
		population.insert("Italy", 60963000);
		population.insert("Spain", 46335000);
		population.insert("Ukraine", 42850000);
		population.insert("Poland", 38494000);
		population.insert("Yugoslavia", 21706000);
		population.insert("Romaina", 19822000);
		population.erase("Yugoslavia");
		alive = true;
	}
	return population;
}

void testIterator(GTree<string, int>& pop){
	for (auto it = pop.begin(); it != pop.outOfRange(); ++it){
		cout << "Population of " << it.key() << " is " << it.value() << endl;
	}
}

void testFind(GTree<string, int>& pop){
	cout << pop.findEqual("Spain").key() << endl;
	cout << (bool)pop.findEqual("Croatia") << endl;
	
	cout << pop.findGreater("Switzerland").key() << endl;
	cout << pop.findGreater("Italy").key() << endl;

	cout << pop.findGreaterEqual("Switzerland").key() << endl;
	cout << pop.findGreaterEqual("Italy").key() << endl;

	cout << pop.findSmaller("Switzerland").key() << endl;
	cout << pop.findSmaller("Italy").key() << endl;

	cout << pop.findSmallerEqual("Switzerland").key() << endl;
	cout << pop.findSmallerEqual("Italy").key() << endl;

	cout << (bool)pop.findGreaterEqual("zz") << endl;
}

void print(int a[], int n){
	int i;
	for (i = 0; i < n-1; i++){
		cout << a[i] << ' ';
	}
	cout << a[i] << endl;
}

void timeTest(void (*f)()){
	int t = clock();
	f();
	double passed = clock() - t;
	cout << passed / CLOCKS_PER_SEC << " seconds" << endl;
}

int main(){
	testIterator(population());
	testFind(population());
	system("pause");
}