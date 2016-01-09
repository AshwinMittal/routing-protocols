SETUP:
The program starts by creating an object called "server" using the MakeFile named make.txt.
Start the program by calling this object on command line and passing the topology file <servername>_init.txt and routing-update-interval (in seconds) as parameters.

1. make -f make.txt
2. ./server -t timberlake_init.txt -i 5 #if running on timberlake and interval time as 5 secs


COMMANDS:
The program automatically send routing updates to its neighbours in the specified interval time.
Apart from this it takes following commands -

1. update <server-ID1> <server-ID2> <Link-Cost> 
#server-ID1 -> ID of server on which the program is running. server-ID2 -> Neighbor server ID. Link-Cost -> New Link Cost
#Link-Cost can be an integer or keyword "inf" to set the cost to infinity.

2. step
#force send the routing updates to neighbours.

3. packets
#display the number of distance vector packets this server has received since the last invocation of this information.

4. display
#display the routing table. Display format -> <destination-server-ID> <next-hop-server-ID> <cost-of-path>

5. disable <server-ID> 
#disable the link to the given server. Can not be restored using "update" command.

6. crash
#close all connections
 

 
