# Example: HTTP Server and Client

This example demonstrates a complete HTTP client-server interaction *entirely within* an Intel SGX enclave. It showcases basic HTTP GET and POST requests, JSON handling, custom filters, and, crucially, the challenges and considerations of making HTTP requests from within a trusted execution environment.

## 1. Example Overview

This example comprises the following components, *all designed to operate inside an SGX enclave*:

*   **HTTP Server (Enclave):** The server code (within `Enclave_t.c` or similar) implements a basic HTTP server using the `ssgx_http_t` library. It handles requests for:
    *   `/hello`: Returns a "Hello" message, optionally customized with a name parameter.
    *   `/time`: Returns the server's current time in JSON format.
    *   `/echo`: Returns a JSON representation of the request (method, path, body, headers).
    *   `/add`: Performs addition of two integers provided in a JSON body.
        The server includes `TimingFilter` and `LoggingFilter` for request timing and logging.

*   **HTTP Client (Enclave):**  The client code (likely within `Enclave_t.c` as well, or in a separate function called from within the enclave) uses the `ssgx_http_t` client library to make HTTP requests to the server. It is *also running inside the SGX enclave*. This means both ends of the HTTP connection are within the trusted environment. The client code demonstrates making GET and POST requests, passing URL parameters, setting headers, and handling responses.

## 2. Compilation

- Configure the Compilation Process

```shell
cmake --preset release-config
```

- Compile sample

```shell
cmake --build --preset release-build --verbose
```

## 3. Running the Service

- Run the http server

```shell
cd release-config/http_server
./host/http_server_app ./enc/http_server_enclave.signed.so
Try to create testing enclave ...
Enclave is created!

Try to run ecall_run_http_server() ...

```

- Run the http client

```shell
cd release-config/http_client
./host/http_client_app ./enc/http_client_enclave.signed.so
Try to create testing enclave ...
Enclave is created!

Try to run ecall_run_http_client() ...

2025-04-01 16:31:09.313[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(34)]ecall_run_http_client:--- GET /hello (no name) ---
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(37)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(38)]ecall_run_http_client:Body: Hello World!
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(44)]ecall_run_http_client:
--- GET /hello?name=TestUser ---
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(49)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(50)]ecall_run_http_client:Body: Hello TestUser!
2025-04-01 16:31:09.314[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(56)]ecall_run_http_client:
--- GET /time ---
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(59)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(60)]ecall_run_http_client:Body: {"server_time":"2025-04-01 08:31:09"}
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(66)]ecall_run_http_client:
--- POST /echo with JSON body ---
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(70)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(71)]ecall_run_http_client:Body: {
    "body": "{\"message\": \"This is a test\"}",
    "headers": {
        "Accept": "*/*",
        "Connection": "Keep-Alive",
        "Content-Length": "29",
        "Content-Type": "application/json",
        "Host": "0.0.0.0:83"
    },
    "method": "POST",
    "path": "/echo"
}
2025-04-01 16:31:09.315[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(77)]ecall_run_http_client:
--- POST /add with JSON body ---
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(81)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(82)]ecall_run_http_client:Body: {"a":10,"b":20,"sum":30}
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(88)]ecall_run_http_client:
--- POST /add with JSON body and custom headers ---
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(95)]ecall_run_http_client:Status: 200
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(96)]ecall_run_http_client:Body: {"a":5,"b":15,"sum":20}
2025-04-01 16:31:09.316[/opt/logs/tee-log/log-safeheron-mpc-engine][INFO][main][140302699816768][Enclave.cpp(101)]ecall_run_http_client:
--- End of Tests ---

Exit from function ecall_run_http_client()!
Destroy enclave!
```

## Security Considerations

- Server Response Encryption and Signing (Inside the Enclave)

    * Receive and Validate the Request (Server Side):* The server receives the data from the OCALL.
    * Verify the Signature:* Use the client's public key (obtained securely, e.g., from the client's certificate or a pre-shared key exchange) to verify the signature over the *encrypted request data* and the associated metadata. *Reject the request immediately if the signature verification fails.*  This step authenticates the *client*.
    * Process the Request and Generate Response:* Perform the necessary operations based on the request. Create the response body (e.g., the result of the addition).
    * Encrypt the Response Body:* Encrypt the response body using a symmetric encryption algorithm with a key derived from the key management step.  Use a unique IV or nonce.
    * Sign the Encrypted Response Data:* Calculate a digital signature over the encrypted response *and* any relevant response metadata (e.g., status code, headers).

- Client Response Decryption and Validation (Inside the Enclave)
    * Receive the Response Data:* The client receives the response data from the OCALL.
    * Verify the Signature:* Use the server's public key (obtained securely) to verify the signature over the *encrypted response data* and any associated metadata. *Reject the response if the signature verification fails.* This authenticates the *server*.
    * Decrypt the Response Body:* Decrypt the response body using the correct key and IV/nonce.

*   **Protection against Replay Attacks:**
    *   Implement a robust replay attack prevention mechanism, such as a:
        *   **Sequence Number:**  Include a sequence number in each request and response and ensure it increases monotonically. Store the last valid sequence number on the server-side and reject any requests with a sequence number lower or equal than it.
        *   **Nonce (Number Used Once):**  Use a nonce that's unique for each request or response, preventing the same message from being used multiple times. The nonce can be random and included with the message.
