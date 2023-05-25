# vegadude

vegadude is a utility application to upload programs on CDAC's VEGA Microprocessors.

It is heavily under development and currently only uploading via Serial + XMODEM (w/ CRC-16-CCITT) has been implemented.

## Clone

```
git clone https://github.com/rnayabed/vegadude.git
```

## Build

```
cmake -B build -S vegadude -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```
./build/vegadude -tp /dev/ttyUSB0 -bp <path to binary> -ssb 1 -sbi 8 -sba 115200 -sau
```

## Note

vegadude does not play well with sniffer programs like [interceptty](https://github.com/geoffmeyers/interceptty), *for now*.
