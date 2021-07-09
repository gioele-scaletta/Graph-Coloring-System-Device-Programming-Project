#include <iostream>

class X {
public: //the five operations are overloaded
 //constructors
	X() { std::cout << "dc " << std::endl; }
	//default constructor dc
	X(const X &) { std::cout << "cc" << std::endl; }
	//copy constructor cc
	X(X &&) noexcept { std::cout << "mc" << std::endl; };
	//move constructor mc
	//assignments
	X &operator=(const X &) { std::cout << "ca" << std::endl; return *this; }
	//copy assignment ca
	X &operator=(X &&) { std::cout << "ma" << std::endl; return *this; }
	//move assignment ma
	//destructor
	~X() { std::cout << "d" << std::endl; }
	//destructor d
};
void f1(X x) { }
void f2(X &x) { }
void f3(X &&x) { }
int main() {
	std::cout << "\n----- X x1, x2;\n";
	X x1, x2;
	std::cout << "\n----- X x3 = x1;\n";
	X x3 = x1;
	std::cout << "\n----- f1(x2);\n";
	f1(x2);
	std::cout << "\n----- f2(x3)\n";
	f2(x3);
	std::cout << "\n----- f3(std::move(x2));\n";
	f3(std::move(x2));
	std::cout << "\n----- END PROGRAM\n";

	system("pause");
	return 0;
}