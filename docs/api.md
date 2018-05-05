API structure for AtomMiner CLI miner
==============

API provides all kind of statistics for the current mining session in json format. I'll keep adding more stats to the output when it feels necessary.

By default API is on and will try to listen on 0.0.0.0:9123 with SSL is disabled, authentification enabled with api:pass. Default port, SSL, authentification, username and password can be changed in atomminer.conf.

To completely disable API, change atomminer.conf with "api" set to false.
To disable authentification (i.e anybody can read stats) change api_login param to false in atomminer.conf file.

API can be accessed with curl or any other tool like following:
```sh
$ curl --user api:pass localhost:9123/
{
    "algos": [
    ],
    "api": "ok",
    "devices": [
    ],
    "stratum": "Disconnected",
    "uptime": 3
}
```
where:
algos   - an array of currently mined algos with hashrates, etc;
devices - an array of currently connected devices;
uptime  - miner uptime in seconds;

Device specific stats are available on per device basis:
```sh
$ curl --user api:pass localhost:9123/dev/{$deviceID}
```

I am a little paranoid when it comes to transferring data over the network and prefer to have every possible bit of data to be encrypted. API server can still be used in non-secure way though. 


Enabling SSL encryption
-----------------------------

To enable SSL encryption you'll need to specify certificate and private key files that will be used by the API server. There's no specific limitations on the SSL certificaes. You can use your Commercial grade SSL cert or create and use self-signed certificate.

Alter atomminer.conf with following settings to use API over SSL:
```js
    "api_ssl": true,
	"api_cert": "api_server.pem",
	"api_key_pass": ""
```

api_ssl enable/disable SSL encryption
api_cert server certificate file. pem must contain both certificate and its private key; full path to the pem file can be included:
    "api_cert": "/var/atomminer/api_server.pem"
api_key_pass field should only be used if serer's private key is password encrypted


Creating self-signed certificate for API server
-----------------------------

Create private key for your self-signed certificate
```sh
$ openssl genrsa -out api_ca.key 4096
```

Create a certificate:
```sh
$ openssl req -new -x509 -days 36500 -key api_ca.key -out api_ca.crt
```

You can check newly generated private key and certificate with the following lines:
```sh
$ openssl rsa -in api_ca.key -check -noout
$ openssl x509 -in api_ca.crt -text -noout
```

api_ca is your Certificate Authority that'll be required by connecting clients in order to verify server certificate validity.
Create and sign server certificate:
```sh
$ openssl genrsa -out api_server.key 4096
$ openssl req -new -key api_server.key -out api_server.csr
$ openssl x509 -req -days 36500 -in api_server.csr -CA api_ca.crt -CAkey api_ca.key -set_serial 01 -out api_server.cert
```

Sample certificate and key are attached.
api_server.cert CN=localhost valid till Apr 10 23:37:21 2118
api_server.key  no password
api_server.pem  no password; CN=localhost; valid till Apr 10 23:37:21 2118

Create self-containing pem container with both server certificate and its private key
```sh
$ cat api_server.cert  api_server.key > api_server.pem
```

Connecting to API over SSL
==============

Make sure HTTPS protocol is used to query API server. And SSL option is set to true in the atomminer.conf file.
```sh
$ curl --user api:pass  https://localhost:9123
{
    "algos": [
    ],
    "api": "ok",
    "devices": [
    ],
    "stratum": "Disconnected",
    "uptime": 19
}
```

Troubleshooting connection
-----------------------------

```sh
$ curl --user api:pass localhost:9123
curl: (52) Empty reply from server
```
Most likely API is configured to use SSL encryption while curl attempts to request data through unsecured connection. https must be used to connect to API endpoint.

```sh
$ curl --cacert api_ca.crt --user api:pass1  https://localhost:9123
403 - Not authorized
```
403 - Not authorized is returned when login/password do not match api_user/api_password in atomminer.conf

```sh
$ curl --cacert api_ca.crt --user api:pass  https://localhost:9123
curl: (35) OpenSSL SSL_connect: SSL_ERROR_SYSCALL in connection to localhost:9123 
```
Most of the time caused by error in certificate/private key file or missing private key. Validate your api_server.pem manually to resolve this problem.

```sh
$ curl --user api:pass  https://localhost:9123
curl: (60) SSL certificate problem: unable to get local issuer certificate
More details here: https://curl.haxx.se/docs/sslcerts.html

curl performs SSL certificate verification by default, using a "bundle"
 of Certificate Authority (CA) public keys (CA certs). If the default
 bundle file isn't adequate, you can specify an alternate file
 using the --cacert option.
If this HTTPS server uses a certificate signed by a CA represented in
 the bundle, the certificate verification probably failed due to a
 problem with the certificate (it might be expired, or the name might
 not match the domain name in the URL).
If you'd like to turn off curl's verification of the certificate, use
 the -k (or --insecure) option.
HTTPS-proxy has similar options --proxy-cacert and --proxy-insecure.
```
Error issued by curl itself due to problem(s) verifying server certificate. If you're using self-signed certificate, you can fix this error just by pointing curl (or whatever service is being used to connect to API) to CA certificate that was used to sign server's cert (api_ca.crt). Or by simply adding your root CA to the well-known certs location (in my case it is /etc/ssl/certs).

Example of valid curl connection with CA explicitly passed to curl:
```sh
$ curl --cacert api_ca.crt --user api:pass  https://localhost:9123
{
    "algos": [
    ],
    "api": "ok",
    "devices": [
    ],
    "stratum": "Disconnected",
    "uptime": 19
}
```
