#!/bin/sh

set -x
# stop on failure
set -e 

days=1024

# generate root key for CA
echo "INFO: generating root key"
openssl genrsa -out root.key 2048

# if [ -f "root.pem" ]; then
#     echo "Current root.pem subject is: "
#     openssl x509 -text -in root.pem  | grep "Subject" | grep C=
# fi

# self sign CA certificate
echo "INFO: generating root certificate"
echo -e "ES\nMadrid\nMadrid\nASPL\nTI Support\nRoot\nsoporte@aspl.es\n" | openssl req -x509 -new -nodes -key root.key -days $days -out root.pem

# join this two certificates into a single file
cat root.pem root.key > root.pem2
mv root.pem2 root.pem
rm -f root.key

# Server certificate
echo "INFO: generate server key"
openssl genrsa -out server.key 2048

echo -e "ES\nMadrid\nMadrid\nASPL\nTI Support\nserver.nopoll.aspl.es\nsoporte@aspl.es\n\n\n" | openssl req -new -key server.key -out server.csr
# openssl req -new -key server.key -out server.csr

# Sign certificate
openssl x509 -req -in server.csr -CA root.pem -CAkey root.pem -CAcreateserial -out server.pem2 -days $days

echo "INFO: copy file"
cat server.pem2 server.key > server.pem
rm server.pem2
rm -f server.key
rm -f server.csr


# Client certificate
echo "INFO: generate client key"
openssl genrsa -out client.key 2048

echo -e "ES\nMadrid\nMadrid\nASPL\nTI Support\nclient.nopoll.aspl.es\nsoporte@aspl.es\n\n\n" | openssl req -new -key client.key -out client.csr
# openssl req -new -key server.key -out server.csr

# Sign certificate
openssl x509 -req -in client.csr -CA root.pem -CAkey root.pem -CAcreateserial -out client.pem2 -days $days

echo "INFO: copy file"
cat client.pem2 client.key > client.pem
rm -f client.pem2
rm -f client.key
rm -f client.csr


