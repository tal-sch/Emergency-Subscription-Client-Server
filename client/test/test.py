import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
port = 7777
sock.bind(('localhost', port))
sock.listen()
print('Listening on port', port)
client_sock, client_addr = sock.accept()
print(client_addr, 'connected')

while True:
    print(client_sock.recv(256))
