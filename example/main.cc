#include <iostream>
#include <cstdlib>
#include "unistd.h"
#include "HelloPrinter.hh"

int main() {

	HelloNamespace::HelloPrinter hey;
	std::cout << "Public version: \n";
	hey.say_hello(2);

	//std::cout << "\nPublic interfaced version: \n";
	//hey.say_hello_interface(5); // say_hello will no longer be interposed here, the compiler uses the HelloPrinter.cc definition

	std::cout << "\nPrivate version: \n";
	hey.private_say_hello_interface(2);

	std::cout << "Testing values: \n";
	std::cout << "public: " << hey.hello_times << "\n";	
	std::cout << "private: " << hey.get_private_say_hello_times() << "\n";	

	std::cout << "main.cc: ";
	std::cout << "Ran successfully!\n";

	sleep(1);
	return 0;
}
