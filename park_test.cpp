#include"park.hpp"
#include<iostream>
using namespace std;

int main(){

	Park park;
	park.status = 1;
	park.controlling();
	park.status = 2;
	park.controlling();
	cout<<"success"<<endl;
		
	return 0;
}
