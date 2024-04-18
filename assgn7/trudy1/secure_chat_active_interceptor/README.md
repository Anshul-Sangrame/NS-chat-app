# NS-chat-app
NS chat app

## General instruction

### Removing all binaries
```bash
make clean
```

### Execution instruction
```bash
make
# for server
./main -s
# for client
./main -c <server-hostname>
# for active interceptor
./main -d <server-hostname> <client-hostname>
```


### Using the Interceptor
- Up/Down arrow - scroll
- Page Up/Down - toggle forwarding/dropping of packets
- Right arrow - copy message to input box
- Left arrow - toggle direction of spoofed message
