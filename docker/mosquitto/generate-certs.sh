#!/bin/sh

set -e

CERTS_DIR="./certs"
DAYS_VALID=3650
COUNTRY="DE"
STATE="Home"
ORG="GrowGrid"
CA_CN="GrowGridCA"
SERVER_CN="mosquitto"
ESP32_CN="esp32-client"
TELEGRAF_CN="telegraf-client"

mkdir -p "$CERTS_DIR"
cd "$CERTS_DIR"

echo "--- Generating Certificate Authority (CA) ---"
openssl genrsa -out ca.key 4096
openssl req -new -x509 -days $DAYS_VALID -key ca.key -out ca.crt -subj "/C=$COUNTRY/ST=$STATE/O=$ORG/CN=$CA_CN"

echo "--- Generating Mosquitto Server Certificate ---"
# Check for SERVER_IP environment variable
if [ -z "$SERVER_IP" ]; then
  echo "Error: SERVER_IP environment variable is not set."
  echo 'Please set it in your .env.dev file.'
  exit 1
fi
echo "Using Server IP: $SERVER_IP"

openssl genrsa -out server.key 2048
openssl req -new -out server.csr -key server.key -subj "/C=$COUNTRY/ST=$STATE/O=$ORG/CN=$SERVER_CN"

# Create the v3.ext file dynamically with the environment variable
cat > v3.ext <<-EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names
[alt_names]
DNS.1 = $SERVER_CN
IP.1 = $SERVER_IP
EOF

openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out server.crt -days $DAYS_VALID -sha256 -extfile v3.ext

echo "--- Generating Telegraf Client Certificate ---"
openssl genrsa -out telegraf.key 2048
openssl req -new -out telegraf.csr -key telegraf.key -subj "/C=$COUNTRY/ST=$STATE/O=$ORG/CN=$TELEGRAF_CN"
openssl x509 -req -in telegraf.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out telegraf.crt -days $DAYS_VALID -sha256

echo "--- Generating ESP32 Client Certificate ---"
openssl genrsa -out esp32.key 2048
openssl req -new -out esp32.csr -key esp32.key -subj "/C=$COUNTRY/ST=$STATE/O=$ORG/CN=$ESP32_CN"
openssl x509 -req -in esp32.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out esp32.crt -days $DAYS_VALID -sha256

echo "--- Setting strict file permissions ---"
chmod 600 *.key

# Clean up intermediate files
rm *.csr
rm v3.ext

echo ""
echo "Success! All certificates and keys have been generated in '$CERTS_DIR'."

echo ""
echo "--- Copying ESP32 certificates to firmware component ---"
FIRMWARE_CERTS_DIR="../../main/components/cert_store/certs"
mkdir -p "$FIRMWARE_CERTS_DIR"
cp ca.crt esp32.crt esp32.key "$FIRMWARE_CERTS_DIR/"
echo "Successfully copied certificates to '$FIRMWARE_CERTS_DIR'."

echo ""
echo "You can now build the firmware and start the Docker services."

