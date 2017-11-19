ECEN 602
Network Programming Assignment # 03

Team No: 01
Ishan Tyagi
Sushrut Kaul

Contributions:


Sushrut did the Server code including Zombie process handling.

Ishan did the testing part via some test cases, file formatting for netascii and octet mode and timeout functioanlity.

Our package for this particular assignment shall contain the following 4 files:
1.tftp_server.c
2.Makefile
3.README.txt
4.Test Cases.pdf

To compile the code, run the Makefile: make
Run the server by using the command line: ./tftp_server localhost server_port
To clean the executables run: make clean
Connecting native client to server: tftp>connect localhost server_port

CLIENT:
We used native linux tftp client for the testing of our tftp server.

SERVER:

We implemented the server code in C. All the processes including the biding to a port and the socket creation were done here. The main function 
included the listening through the speicifc port and after getting the RRQ request from the client, fork() command forks a child process and awaits another
connection. We are tesing few conditions with regard to accessbility in the child process and if the child can pass them then we close the parent socket and 
create a  new socket for data/file transfer. MOst of the perations are done for two types of file formats- NETASCII and OCTET. The file is read 512 bytes at one
time. When we receive the final ACK, the new socket gets cllosed and the server is required to clean up the child.


	













