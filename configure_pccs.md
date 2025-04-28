# Configure PCCS

## 1. For environments utilizing cloud services:

We recommend deploying TEE server on cloud services, as they typically provide a stable and robust environment along with adequate technical support.

```
PCCS_URL=https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v3/
USE_SECURE_CERT=TRUE
```
> **Note:**
> Clear the file first, then set the PCCS_URL value to the address of the PCCS service you are using (for example, if you are using Alibaba Cloud services). If you encounter any issues, please consult your cloud service provider.

```json
{
  "pccs_url": "https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v3/",
  "use_secure_cert": true
}
```

> **Note:**
> You can also use a JSON structure, but please be mindful of the case sensitivity of keywords.

```
PCCS_URL=https://global.acccache.azure.net/sgx/certification/v3/
USE_SECURE_CERT=TRUE
COLLATERAL_SERVICE=https://api.trustedservices.intel.com/sgx/certification/v3/
```

> **Note:**
> Some cloud service PCCS may have expired quote verification collateral (for example, if you are using Microsoft Cloud services). In such cases, you can use Intel PCS or another PCCS to get quote verification collateral.

## 2.  For others:

You can deploy and utilize your own PCCS for remote attestation.

```
PCCS_URL=https://pccs-server-url:8081/sgx/certification/v3/
USE_SECURE_CERT=TRUE
```

For information and deployment details regarding PCCS, please refer to [quote-verification-attestation-with-intel-sgx-dcap](https://www.intel.cn/content/www/cn/zh/developer/articles/technical/quote-verification-attestation-with-intel-sgx-dcap.html?wapkw=dcap).