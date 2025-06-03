## BUS

Client perspective:

A1: LED

A2:  CMD line

C0-7: Open drain A

D0-7: Open drain B


CMD: low

packet start

A0: CLK
A1-A2: 0: RESET, 1: HOST_PACKET
2: CLIENT_PACKET, 3: BROADCAST
A4-A7: PACKET_SIZE (or MAX size). optional

B0-B7: address

packet body lines

A0: CLK
A1-A2: 0: ERROR, 1: 3: NOT_PRESENT
A3: 0: MORE_DATA, 1: EOF
A4 - A7: CRC (optional)

B0-B7: DATA
