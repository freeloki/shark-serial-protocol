This project contains Arduino implementation and a PC implementation of a serial protocol.
The protocol is point to point: no addressing included
The protocol has a CRC16-checksum to check validity of the data
The protocol has been tested with 100byte payload packets and smaller: throughput = 8-9Kbytes/s.
The protocol uses byte stuffing.
The packet format is as follows:

'
\<start flag\>
...
databytes
...
\<CRCflag\>
CRCbyte1
CRCbyte2
\<end flag\>
'