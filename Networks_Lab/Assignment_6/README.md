# Execute

- Go to FTAPServer directory
- Compile the code
    `gcc FTAPServer.c`
- Run the binary
    `./a.out`
- Open a new terminal
- Go to FTAPClient directory
- Compile the code
    `gcc FTAPClient.c`
- Run the binary
    `./a.out`
- Run the commands for the client

# Implementation

- All commands are taken as input from the Client program
- This is then send to the server using socket programming
- Server implements a multi user socket interface
- Each command is processed and functions are called accordingly
- Result is send back to Client as another packet
