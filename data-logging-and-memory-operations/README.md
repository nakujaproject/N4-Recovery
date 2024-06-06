## Encoding and Storage 
---

The aim of this optimization is to ensure we have as much memory as possible to store the flight data. To do this, we need to test how much in-memory space various data formats will take. This is explained below

### Encoding and storing integers and floats
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

I wrote a program to convert a string to HEX format, and compared the following string:
```
flight data
```

In HEX format, it produces the following:

```
666C696768742064617461
```

Raw string size (Windows platform)

![raw-string](./raw-string.png)

Hex string size 

![hex-string](./dec-test.png)

The raw string occupies 11 bytes, while the same strinf in HEX format occupies a whooping 24 bytes! This is not feasible considering we will be storing several strings in a 4 MB limited flash memory over the flight time, which will certainly occupy lots of memory if we store HEX strings.




### References
1. https://www.codeproject.com/Articles/99547/Hex-strings-to-raw-data-and-back
