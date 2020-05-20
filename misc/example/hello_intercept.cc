#include <iostream>
#include <dlfcn.h>
#include "stdio.h"
//#include "HelloPrinter.hh" // this works for getting the right definition, too

namespace HelloNamespace { //same namespace as before, works as a succint way to overwrite say_hello at least
	class HelloPrinter {
		public:
			HelloPrinter();
			void say_hello (int num_times);
		private:
			void private_say_hello(int num_times);
	};
	extern "C" void use_a_printer(void);
}

typedef void (*hello_fcn_ptr)(HelloNamespace::HelloPrinter*,int); // defines a new type hello_fcn_ptr (makes "this" pointer explicit)
typedef void* (*dlopen_ptr)(const char*,int); // typedef for dlopen

HelloNamespace::HelloPrinter* printer_ptr = NULL;

//HelloNamespace::HelloPrinter::HelloPrinter() { // we must interpose this
	//printer_ptr = this;
//}
	
	
	

void HelloNamespace::HelloPrinter::say_hello(int num_times){ // try overwriting a method
		std::cout << "hello_intercept.so: " << "Caught say_hello method\n";
		//Find out what the original function supposed to be called was
		//Have to use mangled name because no 'extern "C"' was used
		hello_fcn_ptr orig_say_hello = (hello_fcn_ptr)dlsym(RTLD_NEXT,"_ZN14HelloNamespace12HelloPrinter9say_helloEi");
		if (orig_say_hello == NULL) {
			fprintf(stderr,"Could not find original say_hello\n");
			exit(1);
		}
		//printf("Found func at %p\n",orig_say_hello);


		//printf("Object at %p\n",printer_ptr);
		//call original function
		orig_say_hello(this,num_times);
		//this->say_hello(num_times);
}

void HelloNamespace::HelloPrinter::private_say_hello(int num_times){ // try overwriting a private method
		std::cout << "hello_intercept.so: " << "Caught private_say_hello method\n";
		//Find out what the original function supposed to be called was
		//Have to use mangled name because no 'extern "C"' was used
		//hello_fcn_ptr orig_say_hello = (hello_fcn_ptr)dlsym(RTLD_NEXT,"HelloNamespace::HelloPrinter::say_hello");


		//call original function
		//orig_say_hello(num_times);
}

extern "C" void HelloNamespace::use_a_printer(void) { // try overwriting a generic function in a namespace
	std::cout << "hello_intercept.so: " << "Caught use_a_printer method\n";
	HelloNamespace::HelloPrinter hey;
	hey.say_hello(5); // simulate interposition
}

/* // same as above code (causes a conflict)
extern "C" void use_a_printer(void) { // try overwriting the same generic function and see if it doesn't need the namespace
	std::cout << "hello_intercept.so: " << "Caught use_a_printer method\n";
	HelloNamespace::HelloPrinter hey;
	hey.say_hello(5); // simulate interposition
}*/

extern "C" unsigned int sleep(unsigned int seconds) { // try overwriting a system function
	printf("intercept.so: intercepted sleep function\n");
	return 0;
}
