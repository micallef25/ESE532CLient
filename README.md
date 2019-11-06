# ESE532CLient

Useage 

./client -s 5 -f filename -ip_address

if not specified

sleep time is 5 
filename is vmlinuz.tar 
ip address is 192.168.0.100

This file breaks up a file and sends them via udp datagrams to your ultra96 board.

Be sure to run ifconfig on both your sender and receiver boards

server: ifconfig eth0 192.168.0.100
client: ifconfig eth0 192.168.0.102

If you are running from pc to board you must be running linux or something that can support sockets. Be sure to also configure the ethernet. So something like

server: ifconfig eth0 192.168.0.100
client(pc) ifconfig <ethernet port> 192.168.0.102

you can run ifconfig by itsefl to figure out the name of your ethernet interface.
