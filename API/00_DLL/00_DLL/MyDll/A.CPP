#include <iostream>
using namespace std;


//extern "C" // c style compile
__declspec(dllexport) // compiler ???þ?..
void foo()
{
	cout << "MyDll::foo version 2" << endl;
}

//extern "C"
__declspec(dllexport)
int Add(int a, int b)
{
	return a+b;
}

