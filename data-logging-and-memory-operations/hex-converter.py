'''
Author: Edwin Mwiti
Email: emwiti658@gmail.com
Date: 10th June 2024

This program is a utility to convert a sentence into HEX format

'''

def show_menu():
	print("\n")
	print("="*40)
	print("HEX CONVERTER")
	print("="*40)

	print("Select option: ")
	print("1. char to HEX")
	print("2. HEX to char (To be done)")
	c = input("Enter choice: ")
	print("\n")
	return c

def split_string(st):
	words = [word + " " for word in st.split()]
	return words

def split_word(w):
	return w.split()

# Driver code
choice = show_menu()
while choice.isdigit() != True:
	print("Please enter the right option")
	choice = show_menu()

match int(choice):
	case 1:
		s = input("Enter string: ")
		result = [] 
		for w in split_string(s):
			for l in w:
				l_conv = hex(ord(l))
				result.append(l_conv[2:])
				res_str = "0x" + "".join(result)
				
		print(res_str)

	# TODO: implement HEX to str

	# case 2:
	# 	s = input("Enter HEX string: ")
	# 	result = [] 
	# 	for w in split_string(s):
	# 		for l in w:
	# 			print(l)
	# 			# l_conv = chr(int(str(l), 16))
	# 			# result.append(l_conv)
				
	# 	# print(result)

