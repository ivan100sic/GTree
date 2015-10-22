#include "GTree.h"
#include <ctime>
#include <iostream>
#include <algorithm>
using namespace gtree;
using namespace std;

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

void testDestructor(int n){
	GTree<int> drvo;
	for (int i = 1; i <= n; i++) drvo.insert(i);
	drvo.erase(n / 2);
	drvo.erase(n / 3);
	drvo.erase(n / 5);
}

void print(int a[], int n){
	int i;
	for (i = 0; i < n-1; i++){
		cout << a[i] << ' ';
	}
	cout << a[i] << endl;
}

int main(){
	int n = 1000000;
	int* a = new int[n];
	for (int i = 0; i < n; i++) a[i] = rand() * rand();

	//print(a, n);

	int t = clock();
	//begin
	gsort(a, n);
	//testDestructor(15);
	//end
	int passed = clock() - t;

	//print(a, n);
	cout << 1.*passed / CLOCKS_PER_SEC << " seconds" << endl;

	delete[] a;
	system("pause");
}