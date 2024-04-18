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
# for passive interceptor
./main -d <server-hostname> <client-hostname>
```

### Using the Interceptor
- Up/Down arrow - scroll

