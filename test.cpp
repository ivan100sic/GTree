#include "GTree.h"
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
using namespace gtree;
using namespace std;
/*
void gsort(int a[], int n){
	GTree<int, int> drvo;
	for (int i = 0; i < n; i++){
		if (!drvo.exists(a[i])){
			drvo.insert(a[i], 1);
		}
		else {
			drvo.insert(a[i], 1 + drvo[a[i]]);
		}
	}

	GTree<int, int> drvo2(drvo);

	for (int i = 0; i < n; i++){
		a[i] = drvo2.minKey();
		if (drvo2[a[i]] == 1)
			drvo2.erase(a[i]);
		else
			drvo2.insert(a[i], drvo2[a[i]] - 1);
	}

	cout << "orig " << drvo.maxKey() << endl;
}
*/
void testDestructor(int n){
	GTree<int> drvo;
	for (int i = 1; i <= n; i++) drvo.insert(i);
	drvo.erase(n / 2);
	drvo.erase(n / 3);
	drvo.erase(n / 5);
}

void testIterator(){
	GTree<string, int> population;
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

	for (auto it = population.begin(); it != population.outOfRange(); ++it){
		cout << "Population of " << it.key() << " is " << it.value() << endl;
	}
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
	testIterator();
	system("pause");
}