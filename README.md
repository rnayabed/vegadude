# vegadude

vegadude is a utility application to upload programs on CDAC's VEGA Microprocessor powered boards.

## Protocols implemented:

* Serial + XMODEM (w/ CRC-16-CCITT) (CDAC Aries)

## Supported Platforms

* Linux
* Windows

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
./build/vegadude -tp /dev/ttyUSB0 -bp <path to binary> --aries -sau
```

## Usage

```
Usage:  [-l | --log] [-bp | --binary-path]
        [-tp | --target-path]
        [-xmr | --xmodem-max-retry] [-xbs | --xmodem-block-size]
        [--aries] [-sp | --serial-parity] [-ssb | --serial-stop-bits]
        [-src | --serial-rts-cts] [-sb | --serial-bits]
        [-sbr | --serial-baud-rate] [-srt | --serial-read-timeout]
        [-sau | --start-after-upload] [--license] [-h | --help]

Option Summary:
    -l | --log                          Optional. Create a log file.

    -bp | --binary-path                 Required. Specify path to the binary file
                                        to be uploaded.

    -tp | --target-path                 Required. Specify path to the target board.

    -xmr | --xmodem-max-retry           Optional. Specify max amount of times to retry before aborting upload.
                                        Default is 10.

    -xbs | --xmodem-block-size          Required if not using automatic configuration.
                                        Specify block size of XModem data transfer.

    --aries                             Use CDAC Aries serial port configuration.

    -sp | --serial-parity               Optional. Specify if target uses parity bit.
                                        Default is false.

    -ssb | --serial-stop-bits           Required. Specify number of stop bits
                                        for target.

    -src | --serial-rts-cts             Optional. Specify if target uses RTS/CTS
                                        flow control.
                                        Default is false.

    -sb | --serial-bits                 Required. Specify the number of data bits sent
                                        in a byte to the target.

    -sbr | --serial-baud-rate           Required. Specify serial baud rate of the
                                        target.

    -srt | --serial-read-timeout        Optional. Specify timeout for each read in
                                        milliseconds.
                                        Default is 500.

    -sau | --start-after-upload         Optional. Immediately start running program
                                        after uploading.

    --license                           Print license information.

    -h | --help                         Print this message.

NOTE: you cannot use --aries and --xmodem-block-size / --serial* arguments (except --serial-read-timeout) at the same time.)
```

## Note

vegadude does not play well with sniffer programs like [interceptty](https://github.com/geoffmeyers/interceptty), *for now*.

## License

vegadude is licensed to [GNU General Public License v3.0](https://github.com/rnayabed/vegadude/blob/master/LICENSE).

```
vegadude - Utility application to upload programs on CDAC's VEGA Microprocessor powered boards.
Copyright (C) 2023 Debayan Sutradhar (rnayabed)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
```

