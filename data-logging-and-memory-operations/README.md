### Encoding and Storage 
Consider the case where you want to store a string like "125" in memory. To store such a string, you need three bytes, and at least one more byte for the length. 

To carry out this comparison, I created two files. One with purely integer values and the other with the same integer values but in HEX format.

```
// decimal-test-file.log
2245454
2245454
2245454
34
34
34
34
34
34
34

```

```
// hex-test-file.log
22434E
22434E
22434E
22
22
22
22
22
22
22

```

Saving and checking the properties of the two files in Windows platform shows the following: 

For the decimal file, the size is 53 bytes

![dec-test](./dec-test.png)

For the hexadecimal file, the size is 50 bytes

![hex-test](./hex-test.png)

The hexadecimal file is 3 bytes smaller as observed. 

### Encoding and storing strings
For strings, since we are not going to use any non-ASCII characters, we can store the string as is. This means that if we have a string like "flight data", it would take 12 bytes, since one char is 1 byte. In addition, the string metadata like length and nul terminator take some few extra bytes. 

If we could encode this string into hex, using UTF-8 encoding scheme, it would take the same 12 bytes. 
This test is carried out below to justify why we store the strings as is without encoding them. 




### References
1. https://www.codeproject.com/Articles/99547/Hex-strings-to-raw-data-and-back
