#include <stdio.h>

int main() {
	float x = 90.4;
	float y = 34.9;
	int f = 34.5;
	char converted_buffer[4];

	// convert to HEX, preserving 4 bytes
	// sprintf(converted_buffer, "%08lx", x);

	printf("%x\n", f);

	return 0;
}