#ifndef __HELLOPRINTER
#define __HELLOPRINTER
namespace HelloNamespace {
	class HelloPrinter {
			public:
				HelloPrinter(); // constructor
				void say_hello(int num_times);
				int hello_times;
				void say_hello_interface(int num_times);
				void private_say_hello_interface(int num_times);
				int get_private_say_hello_times();
			private:
				void private_say_hello(int num_times);
				int private_hello_times;
	};
	extern "C" void use_a_printer(void);
	extern "C" HelloPrinter* create_printer(void);
	extern "C" void delete_printer(HelloPrinter* printer);
}
#endif
