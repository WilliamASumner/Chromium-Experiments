#include <iostream>
#include "unistd.h"
#include "HelloPrinter.hh"

namespace HelloNamespace {

	extern "C" HelloPrinter* create_printer(void) { // extern C'ing these for ease of importing
		return new HelloPrinter;
	}

	extern "C" void delete_printer(HelloPrinter* printer) {
		delete printer;
	}

	HelloPrinter::HelloPrinter() {
		hello_times = 0;
		private_hello_times = 0;
	}


	void HelloPrinter::say_hello(int num_times) { // method to interpose
		printf("Incrementing hello_times, located at %p\n",&hello_times);
		hello_times += num_times;
		for (int i = 0; i < num_times; i++) {
			std::cout << "Hello!\n";
		}
	}
	
	void HelloPrinter::say_hello_interface(int num_times) { // testing if say_hello will still be overridden if not in main_nodlopen.cc
		say_hello(num_times);
	}

	void HelloPrinter::private_say_hello_interface(int num_times) { // private version of  method to interpose
		private_say_hello(num_times);
	}

	void HelloPrinter::private_say_hello(int num_times) { // private version of  method to interpose
		printf("Incrementing private_hello_times, located at %p\n",&hello_times);
		private_hello_times += num_times;
		for (int i = 0; i < num_times; i++) {
			std::cout << "Hello!\n";
		}
	}
	int HelloPrinter::get_private_say_hello_times() {
		return private_hello_times;
	}

	
	extern "C" void use_a_printer(void) { // testing with an indirectly used class
		HelloPrinter hey;
		hey.say_hello(5);
	}
}
