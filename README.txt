Relavent files:

	DieWithError.h: This is a header file that will simply throw an eror message if something unexpected occurs.

	UDPClient.c: This is the file that manages the client side. It creates the socket, send the file it wants to the server, and then will receive the data.

	UDPServer.c: This is the file that manages the server side. It creates the socket, binds to the client, receive the file name and send the client the data from that file.

	makefile: This is the file that allows you to compile the project.

Compilation instructions:

	1.) Use the makefile to remove any lingering files: make clean
	2.) Use the makefile to build all the requies executables: make all
	3.) Start the server side so it is waiting: ./UDPServer <timeout> <loss ratio>
	4.) Start the client side and then it will prompt for file name: ./UDPClient <loss ratio>
	5.) Enter the file you want to reieve from the server: <example.txt>
	6.) The file will be located under out.txt: cat out.txt

Config files:

	makefile: This allows for easy configuations of the project. It is also helpful in cleaning up any lingering files.

Running Instructions:

	UDPServer: This is the executable for the server side. In order to run tyoe ./UDPServer <timeout> <loss ratio>

	UDPClient: This is the executable for the server side. In order to run type ./UDPClient <loss ratio>

	Command line arguments: 
		UDPServer takes the timeout time and the loss ratio for packets

		UDPClient takes the loss ratio for ACKs, the port is hardcoded and the ip address is local.

Testing:
	To Test enter example.txt as the file to copy and a file named out.txt should appear with the same data as in example.txt

