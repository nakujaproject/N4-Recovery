#include <iostream>
#include <cstring>

void strng(char* s) {
	char filename[20];

	strcpy(filename, s);

	std::cout << filename << "\n";

}

int main() {

	char filename[] = "flight1.bin";

	strng(filename);

	return 0;


}