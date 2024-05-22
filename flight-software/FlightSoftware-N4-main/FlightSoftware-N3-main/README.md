# FlightSoftware-N3
Flight software for N3 rocket

## Package installation and update
Inside the directory containing platform.ini, run the following command to update your
packages:

```c
$ pio pkg update
```

### References and Error fixes

1. (Wire LIbrary Device Lock) [Confusing overload of `Wire::begin` · Issue #6616 · espressif/arduino-esp32 · GitHub](https://github.com/espressif/arduino-esp32/issues/6616)