# Example: Millionaire Problem

## Overview

This project demonstrates how to use the Safeheron SGX CMake framework (`ssgx`) to build a sample SGX application that performs **secure comparison** between two private values (the classic *Millionaire Problem*). It shows how Intel SGX and remote attestation can be used to:

- Establish mutual trust between participants via enclave measurement (`MRENCLAVE`)
- Exchange encrypted private inputs with signed authenticity
- Request a third-party Arbiter (an SGX enclave) to compare encrypted values
- Verify the comparison result using SGX remote attestation reports

This example leverages Safeheron's modular `ssgx` build system and can run in Intel SGX hardware or simulation mode.

---

## Build Instructions

### 1. Configure & Build

```bash
cmake --preset release-config
cmake --build --preset release-build
```

This will generate all necessary SGX components, including:

- The host application
- Enclave shared object
- Enclave definition (EDL) and glue code
- Encrypted comparison logic

### 2. Run the Sample

```bash
cd release-config
./host/millionaire_problem_app ./enc/millionaire_problem_enclave.signed.so
```

Expected output:
```
Try to create testing enclave ...
Enclave is created!

alice created a key.public_key=0413a2b933d9018a8c4360f512844d67d598f6b969eda9bcba32d8595a8e2ef2f64567b181dc52a1ca040f223b7bd002e7d7b5d874fb012a1ce42c4f02b1c914e3

bob created a key.public_key=04f1617d28df022775ac3883c060c77a4fbc8bc712f41891f2219a22e03ca8a71cda05edcb8676114046b3e831cf2b1311245ba1de31e874964bf100b1c67913b8

Arbiter prepares:
one time access token: 42fbefa4153c8be370ef33cf3bd3a6e7
remote attestation quote: AwACAAAAAAALABAAk5pyM/ecTKmUCg2zlX8GB9kP9H9I5LI6jtdhxTldqYEAAAAADA4QD///AAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABQAAAAAAAADnAAAAAAAAAPkl1CfglBbNCLSK9TbvBU7TOphgO6Nx6Lo0vChEewRbAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABpKospR3ThsxvYBigJG1DEYfkqs5awkYiJxMnOYl2sZwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACTQmlbFY+YTv/mt1fF5uCUrl9Z6BFo3iDEPGzI9rdx6AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAyhAAAH5ux0raNcCwRPwgaW1mDnp1IDOMaHW4CLxBwZ2jXQTY8iC+F2tymUbmfnaE54J6QjTnpEnJhRVOzPaQARMx20EmgDDPwM/3Ygmc9LRv+L9E8R4c29HwtpKC9XMbMV4LEq2+1CHEJr0jo1EgOoUE+nMKICO0hHxiVypcpy5QT9UsDA4QD///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFQAAAAAAAADnAAAAAAAAAHj+jP0BCVoPEIr/XEBiS5NhLWwotz4ajSgXnJ3fDgaGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACMT1d115ZQPpYTf3fGioKaAFasje1wFAsIGwlEkMV7/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEACwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB5/8FhRE7RLUx02KR6jLAspuSWvmuzeqf+hSr78nCn1AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAi4Cn/hhuphDS+11Wyr9a7ua21SrRzuSGH9XcLTB1aOvKocOSSGmAFdZIRpPTQsl9ZK9BGYuA8PeDj5QREVMDESAAAAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8FAGIOAAAtLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJRThqQ0NCSm1nQXdJQkFnSVZBSVJEN2U4c1Zoc2ptN2JzbVdIK0dJeVJGZkhJTUFvR0NDcUdTTTQ5QkFNQwpNSEF4SWpBZ0JnTlZCQU1NR1VsdWRHVnNJRk5IV0NCUVEwc2dVR3hoZEdadmNtMGdRMEV4R2pBWUJnTlZCQW9NCkVVbHVkR1ZzSUVOdmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUwWVNCRGJHRnlZVEVMTUFrR0ExVUUKQ0F3Q1EwRXhDekFKQmdOVkJBWVRBbFZUTUI0WERUSTFNRFF4TWpFMk1ERXhNbG9YRFRNeU1EUXhNakUyTURFeApNbG93Y0RFaU1DQUdBMVVFQXd3WlNXNTBaV3dnVTBkWUlGQkRTeUJEWlhKMGFXWnBZMkYwWlRFYU1CZ0dBMVVFCkNnd1JTVzUwWld3Z1EyOXljRzl5WVhScGIyNHhGREFTQmdOVkJBY01DMU5oYm5SaElFTnNZWEpoTVFzd0NRWUQKVlFRSURBSkRRVEVMTUFrR0ExVUVCaE1DVlZNd1dUQVRCZ2NxaGtqT1BRSUJCZ2dxaGtqT1BRTUJCd05DQUFUQQpjMk5BQk5LeHJtUjRUdDhXRkdncHZ3VmxhN0FlTDVBOVRIRkpvYTNCbU9tQWh1S1lmR2ZhWk9nY0loaXQ5anRlCkVTV1NJdUFFdHBVVng2L1R2eE9FbzRJRERqQ0NBd293SHdZRFZSMGpCQmd3Rm9BVWxXOWR6YjBiNGVsQVNjblUKOURQT0FWY0wzbFF3YXdZRFZSMGZCR1F3WWpCZ29GNmdYSVphYUhSMGNITTZMeTloY0drdWRISjFjM1JsWkhObApjblpwWTJWekxtbHVkR1ZzTG1OdmJTOXpaM2d2WTJWeWRHbG1hV05oZEdsdmJpOTJNeTl3WTJ0amNtdy9ZMkU5CmNHeGhkR1p2Y20wbVpXNWpiMlJwYm1jOVpHVnlNQjBHQTFVZERnUVdCQlJ4UXBUS0VhdjhjU0lJUFRvUjV4WnQKMCtLcXZEQU9CZ05WSFE4QkFmOEVCQU1DQnNBd0RBWURWUjBUQVFIL0JBSXdBRENDQWpzR0NTcUdTSWI0VFFFTgpBUVNDQWl3d2dnSW9NQjRHQ2lxR1NJYjRUUUVOQVFFRUVITjRmTm9BZ2thTklkREZCR21MYUZNd2dnRmxCZ29xCmhraUcrRTBCRFFFQ01JSUJWVEFRQmdzcWhraUcrRTBCRFFFQ0FRSUJEREFRQmdzcWhraUcrRTBCRFFFQ0FnSUIKRERBUUJnc3Foa2lHK0UwQkRRRUNBd0lCQXpBUUJnc3Foa2lHK0UwQkRRRUNCQUlCQXpBUkJnc3Foa2lHK0UwQgpEUUVDQlFJQ0FQOHdFUVlMS29aSWh2aE5BUTBCQWdZQ0FnRC9NQkFHQ3lxR1NJYjRUUUVOQVFJSEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSUlBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUpBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUsKQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlMQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlNQWdFQU1CQUdDeXFHU0liNApUUUVOQVFJTkFnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJT0FnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJUEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSVFBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSVJBZ0VOTUI4R0N5cUdTSWI0VFFFTkFRSVMKQkJBTURBTUQvLzhBQUFBQUFBQUFBQUFBTUJBR0NpcUdTSWI0VFFFTkFRTUVBZ0FBTUJRR0NpcUdTSWI0VFFFTgpBUVFFQmdCZ2FnQUFBREFQQmdvcWhraUcrRTBCRFFFRkNnRUJNQjRHQ2lxR1NJYjRUUUVOQVFZRUVNUnAxOTF4CkVScHhpVmh0aUNlRUpiOHdSQVlLS29aSWh2aE5BUTBCQnpBMk1CQUdDeXFHU0liNFRRRU5BUWNCQVFIL01CQUcKQ3lxR1NJYjRUUUVOQVFjQ0FRSC9NQkFHQ3lxR1NJYjRUUUVOQVFjREFRSC9NQW9HQ0NxR1NNNDlCQU1DQTBjQQpNRVFDSUUxS0JFVWVnS25KZ3RyS1BMdHlOQzA0eDFsY2lCZ0FFWXdEV1R4YVRTZ1RBaUEvUy9GNUU1dE1vTmhICno4bmhlTWNzRFhQYlBRcHMxeHBJckRtYUduT1RFZz09Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0KLS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNsakNDQWoyZ0F3SUJBZ0lWQUpWdlhjMjlHK0hwUUVuSjFQUXp6Z0ZYQzk1VU1Bb0dDQ3FHU000OUJBTUMKTUdneEdqQVlCZ05WQkFNTUVVbHVkR1ZzSUZOSFdDQlNiMjkwSUVOQk1Sb3dHQVlEVlFRS0RCRkpiblJsYkNCRApiM0p3YjNKaGRHbHZiakVVTUJJR0ExVUVCd3dMVTJGdWRHRWdRMnhoY21FeEN6QUpCZ05WQkFnTUFrTkJNUXN3C

alice Save Arbiter public key succeeded

bob Save Arbiter public key succeeded

alice signing and encryption succeeded

bob signing and encryption succeeded

Arbiter performs a comparison:
one time access token: 42fbefa4153c8be370ef33cf3bd3a6e7
remote attestation quote: AwACAAAAAAALABAAk5pyM/ecTKmUCg2zlX8GB9kP9H9I5LI6jtdhxTldqYEAAAAADA4QD///AAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABQAAAAAAAADnAAAAAAAAAPkl1CfglBbNCLSK9TbvBU7TOphgO6Nx6Lo0vChEewRbAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABpKospR3ThsxvYBigJG1DEYfkqs5awkYiJxMnOYl2sZwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB1lOIUpNTd393jClwdnDPaNMF8MIhyI9YI2hxfB1197QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAyhAAAA2NRj2Id5P4ASpUlBLbsiGx78hgtQrG4pNzkTF6bahQ6g08Mnnn8tzxlUJ5H+l21XSmEavaY7BUYMd/d6lZLJ8mgDDPwM/3Ygmc9LRv+L9E8R4c29HwtpKC9XMbMV4LEq2+1CHEJr0jo1EgOoUE+nMKICO0hHxiVypcpy5QT9UsDA4QD///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFQAAAAAAAADnAAAAAAAAAHj+jP0BCVoPEIr/XEBiS5NhLWwotz4ajSgXnJ3fDgaGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACMT1d115ZQPpYTf3fGioKaAFasje1wFAsIGwlEkMV7/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEACwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB5/8FhRE7RLUx02KR6jLAspuSWvmuzeqf+hSr78nCn1AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAi4Cn/hhuphDS+11Wyr9a7ua21SrRzuSGH9XcLTB1aOvKocOSSGmAFdZIRpPTQsl9ZK9BGYuA8PeDj5QREVMDESAAAAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8FAGIOAAAtLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJRThqQ0NCSm1nQXdJQkFnSVZBSVJEN2U4c1Zoc2ptN2JzbVdIK0dJeVJGZkhJTUFvR0NDcUdTTTQ5QkFNQwpNSEF4SWpBZ0JnTlZCQU1NR1VsdWRHVnNJRk5IV0NCUVEwc2dVR3hoZEdadmNtMGdRMEV4R2pBWUJnTlZCQW9NCkVVbHVkR1ZzSUVOdmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUwWVNCRGJHRnlZVEVMTUFrR0ExVUUKQ0F3Q1EwRXhDekFKQmdOVkJBWVRBbFZUTUI0WERUSTFNRFF4TWpFMk1ERXhNbG9YRFRNeU1EUXhNakUyTURFeApNbG93Y0RFaU1DQUdBMVVFQXd3WlNXNTBaV3dnVTBkWUlGQkRTeUJEWlhKMGFXWnBZMkYwWlRFYU1CZ0dBMVVFCkNnd1JTVzUwWld3Z1EyOXljRzl5WVhScGIyNHhGREFTQmdOVkJBY01DMU5oYm5SaElFTnNZWEpoTVFzd0NRWUQKVlFRSURBSkRRVEVMTUFrR0ExVUVCaE1DVlZNd1dUQVRCZ2NxaGtqT1BRSUJCZ2dxaGtqT1BRTUJCd05DQUFUQQpjMk5BQk5LeHJtUjRUdDhXRkdncHZ3VmxhN0FlTDVBOVRIRkpvYTNCbU9tQWh1S1lmR2ZhWk9nY0loaXQ5anRlCkVTV1NJdUFFdHBVVng2L1R2eE9FbzRJRERqQ0NBd293SHdZRFZSMGpCQmd3Rm9BVWxXOWR6YjBiNGVsQVNjblUKOURQT0FWY0wzbFF3YXdZRFZSMGZCR1F3WWpCZ29GNmdYSVphYUhSMGNITTZMeTloY0drdWRISjFjM1JsWkhObApjblpwWTJWekxtbHVkR1ZzTG1OdmJTOXpaM2d2WTJWeWRHbG1hV05oZEdsdmJpOTJNeTl3WTJ0amNtdy9ZMkU5CmNHeGhkR1p2Y20wbVpXNWpiMlJwYm1jOVpHVnlNQjBHQTFVZERnUVdCQlJ4UXBUS0VhdjhjU0lJUFRvUjV4WnQKMCtLcXZEQU9CZ05WSFE4QkFmOEVCQU1DQnNBd0RBWURWUjBUQVFIL0JBSXdBRENDQWpzR0NTcUdTSWI0VFFFTgpBUVNDQWl3d2dnSW9NQjRHQ2lxR1NJYjRUUUVOQVFFRUVITjRmTm9BZ2thTklkREZCR21MYUZNd2dnRmxCZ29xCmhraUcrRTBCRFFFQ01JSUJWVEFRQmdzcWhraUcrRTBCRFFFQ0FRSUJEREFRQmdzcWhraUcrRTBCRFFFQ0FnSUIKRERBUUJnc3Foa2lHK0UwQkRRRUNBd0lCQXpBUUJnc3Foa2lHK0UwQkRRRUNCQUlCQXpBUkJnc3Foa2lHK0UwQgpEUUVDQlFJQ0FQOHdFUVlMS29aSWh2aE5BUTBCQWdZQ0FnRC9NQkFHQ3lxR1NJYjRUUUVOQVFJSEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSUlBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUpBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUsKQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlMQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlNQWdFQU1CQUdDeXFHU0liNApUUUVOQVFJTkFnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJT0FnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJUEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSVFBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSVJBZ0VOTUI4R0N5cUdTSWI0VFFFTkFRSVMKQkJBTURBTUQvLzhBQUFBQUFBQUFBQUFBTUJBR0NpcUdTSWI0VFFFTkFRTUVBZ0FBTUJRR0NpcUdTSWI0VFFFTgpBUVFFQmdCZ2FnQUFBREFQQmdvcWhraUcrRTBCRFFFRkNnRUJNQjRHQ2lxR1NJYjRUUUVOQVFZRUVNUnAxOTF4CkVScHhpVmh0aUNlRUpiOHdSQVlLS29aSWh2aE5BUTBCQnpBMk1CQUdDeXFHU0liNFRRRU5BUWNCQVFIL01CQUcKQ3lxR1NJYjRUUUVOQVFjQ0FRSC9NQkFHQ3lxR1NJYjRUUUVOQVFjREFRSC9NQW9HQ0NxR1NNNDlCQU1DQTBjQQpNRVFDSUUxS0JFVWVnS25KZ3RyS1BMdHlOQzA0eDFsY2lCZ0FFWXdEV1R4YVRTZ1RBaUEvUy9GNUU1dE1vTmhICno4bmhlTWNzRFhQYlBRcHMxeHBJckRtYUduT1RFZz09Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0KLS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNsakNDQWoyZ0F3SUJBZ0lWQUpWdlhjMjlHK0hwUUVuSjFQUXp6Z0ZYQzk1VU1Bb0dDQ3FHU000OUJBTUMKTUdneEdqQVlCZ05WQkFNTUVVbHVkR1ZzSUZOSFdDQlNiMjkwSUVOQk1Sb3dHQVlEVlFRS0RCRkpiblJsYkNCRApiM0p3YjNKaGRHbHZiakVVTUJJR0ExVUVCd3dMVTJGdWRHRWdRMnhoY21FeEN6QUpCZ05WQkFnTUFrTkJNUXN3C

alice presentation results:Alice is not richer than Bob

bob presentation results:Alice is not richer than Bob

Exit from function
End!
```

---

## Notes

- Requires Intel SGX SDK installed at `/opt/intel/sgxsdk`
- Requires Safeheron SGX Development Framework installed at `/opt/safeheron/ssgx`
- Requires SGX-compatible hardware or simulation driver
- The comparison is performed inside an SGX enclave, and the result is accompanied by a remote attestation report
- The comparison result is only accepted if both Alice and Bob **verify** the quote report and the `MRENCLAVE` of the Arbiter
- This sample demonstrates a typical *confidential MPC-like* flow secured by TEE