This is a simple server-client program for posix systems.

From the client side users can request path from start point to destination point on the given graph.

After make you can run programs with these commands;

For server;
 
 ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 0

For client;

./client -a 127.0.0.1 -p PORT -s 768 -d 979
