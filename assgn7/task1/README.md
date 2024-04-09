# Creation of 512-bit ECC key
	openssl ecparam -name prime256v1 -genkey -noout -out root-key.pem
	openssl ec -in root-key.pem -pubout -out pub-root-key.pem
# Creating of self-signed root certificate

	openssl req -new -x509 -key root-key.pem -out root-cert.crt -days 3650 \
	    -subj "/C=IN/ST=Telangana/O=iTS Root/OU=iTS/CN=iTS Root R1" \
	    -extensions v3_ca \
	    -config <(cat <<-EOF
	[v3_ca]
	subjectKeyIdentifier=hash
	basicConstraints=critical,CA:true
	keyUsage=critical,digitalSignature,cRLSign,keyCertSign
	EOF
	)

# Generating RSA key pair for intermediate certificate generation
	openssl genrsa -out inter-key.pem 4096
	openssl rsa -in inter-key.pem -out pub-inter-key.pem

# Generating certificate signing request
	openssl req -new -key inter-key.pem -out inter-csr.csr \
	-subj "/C=IN/ST=Telangana/O=iTS CA/OU=iTS/CN=iTS CA 1R3" \
	-addext "subjectKeyIdentifier=hash" \
	-addext "extendedKeyUsage=serverAuth,clientAuth" \
	-addext "basicConstraints=critical,CA:true,pathlen:0" \
	-addext "keyUsage=critical,digitalSignature,cRLSign,keyCertSign"

# Generating Intermediate certificate signed by root
	openssl x509 -req -in inter-csr.csr -CA root-cert.crt -CAkey root-key.pem -out inter-cert.crt -days 365 \
    -extfile <(echo -e "extendedKeyUsage = serverAuth, clientAuth\nbasicConstraints = critical, CA:true, pathlen:0\nkeyUsage = critical, digitalSignature, cRLSign, keyCertSign")

# Generating key pair for alice
	openssl genrsa -out alice-key.pem 2048
	openssl rsa -in alice-key.pem -out pub-alice-key.pem

# Generating key pari for bob
	openssl ecparam -name prime256v1 -genkey -noout -out bob-key.pem
	openssl ec -in bob-key.pem -pubout -out pub-bob-key.pem


# Generating CSR for alice
	openssl req -new -key alice-key.pem -out alice-csr.csr \
	-subj "/C=IN/ST=Telangana/O=Alice/OU=Alice/CN=Alice1.com" \
	-addext "nsCertType=client"\
	-addext "subjectKeyIdentifier=hash" \
	-addext "extendedKeyUsage=clientAuth" \
	-addext "basicConstraints=CA:FALSE" \
	-addext "keyUsage=critical,nonRepudiation,digitalSignature,keyEncipherment"

# Signing CSR of alice
	openssl x509 -req -in alice-csr.csr -CA inter-cert.crt -CAkey inter-key.pem -out alice-cert.crt -days 365     -extfile <(echo -e "extendedKeyUsage=clientAuth\nbasicConstraints=CA:FALSE\nkeyUsage=critical,nonRepudiation,digitalSignature,keyEncipherment\nsubjectKeyIdentifier=hash\nnsCertType=client")

# Generating CSR for bob
	openssl req -new -key bob-key.pem -out bob-csr.csr \
        -subj "/C=IN/ST=Telangana/O=Bob/OU=Bob/CN=Bob1.com" \
        -addext "nsCertType=client"\
        -addext "subjectKeyIdentifier=hash" \
        -addext "extendedKeyUsage=clientAuth" \
        -addext "basicConstraints=CA:FALSE" \
        -addext "keyUsage=critical,nonRepudiation,digitalSignature,keyEncipherment"
# Signing CSR of bob
	openssl x509 -req -in bob-csr.csr -CA inter-cert.crt -CAkey inter-key.pem -out bob-cert.crt -days 365     -extfile <(echo -e "extendedKeyUsage=clientAuth\nbasicConstraints=CA:FALSE\nkeyUsage=critical,nonRepudiation,digitalSignature,keyEncipherment\nsubjectKeyIdentifier=hash\nnsCertType=client")

Reference link:`https://openssl-ca.readthedocs.io/en/latest/create-the-root-pair.html`
