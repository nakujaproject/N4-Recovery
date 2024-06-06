// program to convert a string to HEX
// convert each string char to decimal 
// convert this decimal ASCII value to its HEX equivalent

#include <iostream>

std::string decToHEX(int n) {
	char hexNum[100]; // to store hex number

	int i = 0;
	while(n != 0) {
		int temp = 0;
		temp = n % 16;

		if(temp < 10) {
			hexNum[i] = temp + 48;
			i++;
		} else {
			hexNum[i] = temp + 55;
			i++;
		}

		n = n / 16;
	}

	std::string ans = "";

	// printing the hex number in reverse order
	for(int j = i - 1; j >= 0; j--) {
		ans += hexNum[j];
	}

	return ans;
}


std::string ASCIIToHEX(std::string ascii) {
	std::string hex = "";

	// iterate through each character of the ascii string
	for(int i = 0; i < ascii.length(); i++) {
		// take char from position i in the string
		char ch = ascii[i];

		// cast the char to integer value and find its ASCII value
		int tmp = (int) ch;

		// change the ASCII integer value to hexadecimal avalue;
		std::string part = decToHEX(tmp);

		// add to final HEX string
		hex += part;

	}

	return hex;
}

int main() {
	// driver code 
	std::cout << ASCIIToHEX("flight data");
}