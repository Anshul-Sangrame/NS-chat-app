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
./main -m <server-hostname> <client-hostname>
```

### Pending
- Get host address by name


### Using the Interceptor
- Up/Down arrow - scroll