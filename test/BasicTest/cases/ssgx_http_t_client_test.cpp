#include <cstring>
#include <stdexcept>

#include "nlohmann/json.hpp"

#include "ssgx_http_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

/**
 * @brief Test data for httpbin
 *
 */
const std::string httpbin_host = "https://httpbin.org";
const uint16_t httpbin_port = 443;
const std::string httpbin_path_get = "/get";
const std::string httpbin_path_post = "/post";
const std::string httpbin_ca = "-----BEGIN CERTIFICATE-----\n"
                               "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
                               "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                               "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
                               "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                               "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                               "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                               "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                               "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                               "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                               "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                               "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
                               "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
                               "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
                               "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
                               "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
                               "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
                               "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
                               "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN CERTIFICATE-----\n"
                               "MIIEXjCCA0agAwIBAgITB3MSSkvL1E7HtTvq8ZSELToPoTANBgkqhkiG9w0BAQsF\n"
                               "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                               "b24gUm9vdCBDQSAxMB4XDTIyMDgyMzIyMjUzMFoXDTMwMDgyMzIyMjUzMFowPDEL\n"
                               "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEcMBoGA1UEAxMTQW1hem9uIFJT\n"
                               "QSAyMDQ4IE0wMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALtDGMZa\n"
                               "qHneKei1by6+pUPPLljTB143Si6VpEWPc6mSkFhZb/6qrkZyoHlQLbDYnI2D7hD0\n"
                               "sdzEqfnuAjIsuXQLG3A8TvX6V3oFNBFVe8NlLJHvBseKY88saLwufxkZVwk74g4n\n"
                               "WlNMXzla9Y5F3wwRHwMVH443xGz6UtGSZSqQ94eFx5X7Tlqt8whi8qCaKdZ5rNak\n"
                               "+r9nUThOeClqFd4oXych//Rc7Y0eX1KNWHYSI1Nk31mYgiK3JvH063g+K9tHA63Z\n"
                               "eTgKgndlh+WI+zv7i44HepRZjA1FYwYZ9Vv/9UkC5Yz8/yU65fgjaE+wVHM4e/Yy\n"
                               "C2osrPWE7gJ+dXMCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYD\n"
                               "VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNV\n"
                               "HQ4EFgQUwDFSzVpQw4J8dHHOy+mc+XrrguIwHwYDVR0jBBgwFoAUhBjMhTTsvAyU\n"
                               "lC4IWZzHshBOCggwewYIKwYBBQUHAQEEbzBtMC8GCCsGAQUFBzABhiNodHRwOi8v\n"
                               "b2NzcC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbTA6BggrBgEFBQcwAoYuaHR0cDov\n"
                               "L2NydC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbS9yb290Y2ExLmNlcjA/BgNVHR8E\n"
                               "ODA2MDSgMqAwhi5odHRwOi8vY3JsLnJvb3RjYTEuYW1hem9udHJ1c3QuY29tL3Jv\n"
                               "b3RjYTEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqGSIb3DQEBCwUAA4IB\n"
                               "AQAtTi6Fs0Azfi+iwm7jrz+CSxHH+uHl7Law3MQSXVtR8RV53PtR6r/6gNpqlzdo\n"
                               "Zq4FKbADi1v9Bun8RY8D51uedRfjsbeodizeBB8nXmeyD33Ep7VATj4ozcd31YFV\n"
                               "fgRhvTSxNrrTlNpWkUk0m3BMPv8sg381HhA6uEYokE5q9uws/3YkKqRiEz3TsaWm\n"
                               "JqIRZhMbgAfp7O7FUwFIb7UIspogZSKxPIWJpxiPo3TcBambbVtQOcNRWz5qCQdD\n"
                               "slI2yayq0n2TXoHyNCLEH8rpsJRVILFsg0jc7BaFrMnF462+ajSehgj12IidNeRN\n"
                               "4zl+EoNaWdpnWndvSpAEkq2P\n"
                               "-----END CERTIFICATE-----";

/**
 * @brief Test data for postman-echo
 *
 */
const std::string postman_host = "https://postman-echo.com";
const uint16_t postman_port = 443;
const std::string postman_path_get = "/get";
const std::string postman_path_post = "/post";
const std::string postman_ca = "-----BEGIN CERTIFICATE-----\n"
                               "MIIEXjCCA0agAwIBAgITB3MSSkvL1E7HtTvq8ZSELToPoTANBgkqhkiG9w0BAQsF\n"
                               "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                               "b24gUm9vdCBDQSAxMB4XDTIyMDgyMzIyMjUzMFoXDTMwMDgyMzIyMjUzMFowPDEL\n"
                               "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEcMBoGA1UEAxMTQW1hem9uIFJT\n"
                               "QSAyMDQ4IE0wMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALtDGMZa\n"
                               "qHneKei1by6+pUPPLljTB143Si6VpEWPc6mSkFhZb/6qrkZyoHlQLbDYnI2D7hD0\n"
                               "sdzEqfnuAjIsuXQLG3A8TvX6V3oFNBFVe8NlLJHvBseKY88saLwufxkZVwk74g4n\n"
                               "WlNMXzla9Y5F3wwRHwMVH443xGz6UtGSZSqQ94eFx5X7Tlqt8whi8qCaKdZ5rNak\n"
                               "+r9nUThOeClqFd4oXych//Rc7Y0eX1KNWHYSI1Nk31mYgiK3JvH063g+K9tHA63Z\n"
                               "eTgKgndlh+WI+zv7i44HepRZjA1FYwYZ9Vv/9UkC5Yz8/yU65fgjaE+wVHM4e/Yy\n"
                               "C2osrPWE7gJ+dXMCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYD\n"
                               "VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNV\n"
                               "HQ4EFgQUwDFSzVpQw4J8dHHOy+mc+XrrguIwHwYDVR0jBBgwFoAUhBjMhTTsvAyU\n"
                               "lC4IWZzHshBOCggwewYIKwYBBQUHAQEEbzBtMC8GCCsGAQUFBzABhiNodHRwOi8v\n"
                               "b2NzcC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbTA6BggrBgEFBQcwAoYuaHR0cDov\n"
                               "L2NydC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbS9yb290Y2ExLmNlcjA/BgNVHR8E\n"
                               "ODA2MDSgMqAwhi5odHRwOi8vY3JsLnJvb3RjYTEuYW1hem9udHJ1c3QuY29tL3Jv\n"
                               "b3RjYTEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqGSIb3DQEBCwUAA4IB\n"
                               "AQAtTi6Fs0Azfi+iwm7jrz+CSxHH+uHl7Law3MQSXVtR8RV53PtR6r/6gNpqlzdo\n"
                               "Zq4FKbADi1v9Bun8RY8D51uedRfjsbeodizeBB8nXmeyD33Ep7VATj4ozcd31YFV\n"
                               "fgRhvTSxNrrTlNpWkUk0m3BMPv8sg381HhA6uEYokE5q9uws/3YkKqRiEz3TsaWm\n"
                               "JqIRZhMbgAfp7O7FUwFIb7UIspogZSKxPIWJpxiPo3TcBambbVtQOcNRWz5qCQdD\n"
                               "slI2yayq0n2TXoHyNCLEH8rpsJRVILFsg0jc7BaFrMnF462+ajSehgj12IidNeRN\n"
                               "4zl+EoNaWdpnWndvSpAEkq2P\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN CERTIFICATE-----\n"
                               "MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF\n"
                               "ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj\n"
                               "b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x\n"
                               "OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1\n"
                               "dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL\n"
                               "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                               "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                               "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                               "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                               "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                               "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                               "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                               "jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/\n"
                               "BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW\n"
                               "gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH\n"
                               "MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH\n"
                               "MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy\n"
                               "MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0\n"
                               "LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF\n"
                               "AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW\n"
                               "MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma\n"
                               "eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK\n"
                               "bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN\n"
                               "0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U\n"
                               "akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN CERTIFICATE-----\n"
                               "MIIEdTCCA12gAwIBAgIJAKcOSkw0grd/MA0GCSqGSIb3DQEBCwUAMGgxCzAJBgNV\n"
                               "BAYTAlVTMSUwIwYDVQQKExxTdGFyZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTIw\n"
                               "MAYDVQQLEylTdGFyZmllbGQgQ2xhc3MgMiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\n"
                               "eTAeFw0wOTA5MDIwMDAwMDBaFw0zNDA2MjgxNzM5MTZaMIGYMQswCQYDVQQGEwJV\n"
                               "UzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UE\n"
                               "ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjE7MDkGA1UEAxMyU3RhcmZp\n"
                               "ZWxkIFNlcnZpY2VzIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n"
                               "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVDDrEKvlO4vW+GZdfjohTsR8/\n"
                               "y8+fIBNtKTrID30892t2OGPZNmCom15cAICyL1l/9of5JUOG52kbUpqQ4XHj2C0N\n"
                               "Tm/2yEnZtvMaVq4rtnQU68/7JuMauh2WLmo7WJSJR1b/JaCTcFOD2oR0FMNnngRo\n"
                               "Ot+OQFodSk7PQ5E751bWAHDLUu57fa4657wx+UX2wmDPE1kCK4DMNEffud6QZW0C\n"
                               "zyyRpqbn3oUYSXxmTqM6bam17jQuug0DuDPfR+uxa40l2ZvOgdFFRjKWcIfeAg5J\n"
                               "Q4W2bHO7ZOphQazJ1FTfhy/HIrImzJ9ZVGif/L4qL8RVHHVAYBeFAlU5i38FAgMB\n"
                               "AAGjgfAwge0wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0O\n"
                               "BBYEFJxfAN+qAdcwKziIorhtSpzyEZGDMB8GA1UdIwQYMBaAFL9ft9HO3R+G9FtV\n"
                               "rNzXEMIOqYjnME8GCCsGAQUFBwEBBEMwQTAcBggrBgEFBQcwAYYQaHR0cDovL28u\n"
                               "c3MyLnVzLzAhBggrBgEFBQcwAoYVaHR0cDovL3guc3MyLnVzL3guY2VyMCYGA1Ud\n"
                               "HwQfMB0wG6AZoBeGFWh0dHA6Ly9zLnNzMi51cy9yLmNybDARBgNVHSAECjAIMAYG\n"
                               "BFUdIAAwDQYJKoZIhvcNAQELBQADggEBACMd44pXyn3pF3lM8R5V/cxTbj5HD9/G\n"
                               "VfKyBDbtgB9TxF00KGu+x1X8Z+rLP3+QsjPNG1gQggL4+C/1E2DUBc7xgQjB3ad1\n"
                               "l08YuW3e95ORCLp+QCztweq7dp4zBncdDQh/U90bZKuCJ/Fp1U1ervShw3WnWEQt\n"
                               "8jxwmKy6abaVd38PMV4s/KCHOkdp8Hlf9BRUpJVeEXgSYCfOn8J3/yNTd126/+pZ\n"
                               "59vPr5KW7ySaNRB6nJHGDn2Z9j8Z3/VyVOEVqQdZe4O/Ui5GjLIAZHYcSNPYeehu\n"
                               "VsyuLAOQ1xk4meTKCRlb/weWsKh/NEnfVqn3sF/tM+2MR7cwA130A4w=\n"
                               "-----END CERTIFICATE-----";

/**
 * @brief Test data for Binance
 *
 */
const char* binance_host = "https://api1.binance.com";
const int binance_port = 443;
const char* binance_account_path = "/api/v3/account";
const char* binance_ca = "-----BEGIN CERTIFICATE-----\n"
                         "MIIEjTCCA3WgAwIBAgIQDQd4KhM/xvmlcpbhMf/ReTANBgkqhkiG9w0BAQsFADBh\n"
                         "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                         "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
                         "MjAeFw0xNzExMDIxMjIzMzdaFw0yNzExMDIxMjIzMzdaMGAxCzAJBgNVBAYTAlVT\n"
                         "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
                         "b20xHzAdBgNVBAMTFkdlb1RydXN0IFRMUyBSU0EgQ0EgRzEwggEiMA0GCSqGSIb3\n"
                         "DQEBAQUAA4IBDwAwggEKAoIBAQC+F+jsvikKy/65LWEx/TMkCDIuWegh1Ngwvm4Q\n"
                         "yISgP7oU5d79eoySG3vOhC3w/3jEMuipoH1fBtp7m0tTpsYbAhch4XA7rfuD6whU\n"
                         "gajeErLVxoiWMPkC/DnUvbgi74BJmdBiuGHQSd7LwsuXpTEGG9fYXcbTVN5SATYq\n"
                         "DfbexbYxTMwVJWoVb6lrBEgM3gBBqiiAiy800xu1Nq07JdCIQkBsNpFtZbIZhsDS\n"
                         "fzlGWP4wEmBQ3O67c+ZXkFr2DcrXBEtHam80Gp2SNhou2U5U7UesDL/xgLK6/0d7\n"
                         "6TnEVMSUVJkZ8VeZr+IUIlvoLrtjLbqugb0T3OYXW+CQU0kBAgMBAAGjggFAMIIB\n"
                         "PDAdBgNVHQ4EFgQUlE/UXYvkpOKmgP792PkA76O+AlcwHwYDVR0jBBgwFoAUTiJU\n"
                         "IBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsG\n"
                         "AQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMDQGCCsGAQUFBwEB\n"
                         "BCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQuY29tMEIGA1Ud\n"
                         "HwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEds\n"
                         "b2JhbFJvb3RHMi5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEW\n"
                         "HGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwDQYJKoZIhvcNAQELBQADggEB\n"
                         "AIIcBDqC6cWpyGUSXAjjAcYwsK4iiGF7KweG97i1RJz1kwZhRoo6orU1JtBYnjzB\n"
                         "c4+/sXmnHJk3mlPyL1xuIAt9sMeC7+vreRIF5wFBC0MCN5sbHwhNN1JzKbifNeP5\n"
                         "ozpZdQFmkCo+neBiKR6HqIA+LMTMCMMuv2khGGuPHmtDze4GmEGZtYLyF8EQpa5Y\n"
                         "jPuV6k2Cr/N3XxFpT3hRpt/3usU/Zb9wfKPtWpoznZ4/44c1p9rzFcZYrWkj3A+7\n"
                         "TNBJE0GmP2fhXhP1D/XVfIW/h0yCJGEiV9Glm/uGOa3DXHlmbAcxSyCRraG+ZBkA\n"
                         "7h4SeM6Y8l/7MBRpPCz6l8Y=\n"
                         "-----END CERTIFICATE-----\n"
                         "-----BEGIN CERTIFICATE-----\n"
                         "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
                         "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                         "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
                         "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
                         "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
                         "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
                         "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
                         "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
                         "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
                         "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
                         "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
                         "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
                         "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
                         "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
                         "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
                         "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
                         "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
                         "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
                         "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
                         "MrY=\n"
                         "-----END CERTIFICATE-----";

void Test_Constructor_HostOnly(const std::string& host, const std::string& path) {
    ssgx::http_t::Client client(host);
    ssgx::http_t::Result result = client.Get(path);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Constructor_HostOnly) {
    Test_Constructor_HostOnly(httpbin_host, httpbin_path_get);
    Test_Constructor_HostOnly(postman_host, postman_path_get);
}

void Test_Constructor_HostPort(const std::string& host, int port, const std::string& path) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Get(path);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Constructor_HostPort) {
    Test_Constructor_HostPort(httpbin_host, httpbin_port, httpbin_path_get);
    Test_Constructor_HostPort(postman_host, postman_port, postman_path_get);
}

void Test_Constructor_HostPort(const std::string& host, int port, const std::string& ca, const std::string& path) {
    ssgx::http_t::Client client(host, port, ca);
    ssgx::http_t::Result result = client.Get(path);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Constructor_HostPortServerCA) {
    Test_Constructor_HostPort(httpbin_host, httpbin_port, httpbin_ca, httpbin_path_get);
    Test_Constructor_HostPort(postman_host, postman_port, postman_ca, postman_path_get);
}

void Test_Get_PathOnly(const std::string& host, int port, const std::string& path) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Get(path);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Get_PathOnly) {
    Test_Get_PathOnly(httpbin_host, httpbin_port, httpbin_path_get);
    Test_Get_PathOnly(postman_host, postman_port, postman_path_get);
}

void Test_Get_WithHeaders(const std::string& host, int port, const std::string& path,
                          const ssgx::http_t::TypeHeaders & headers) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Get(path, headers);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Get_WithHeaders) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    Test_Get_WithHeaders(httpbin_host, httpbin_port, httpbin_path_get, headers);
    Test_Get_WithHeaders(postman_host, postman_port, postman_path_get, headers);
}

void Test_Get_WithParams(const std::string& host, int port, const std::string& path,
                         const ssgx::http_t::TypeParams& params) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Get(path, params);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Get_WithParams) {
    // Case 1: Pass the path and paramteters separately.
    ssgx::http_t::TypeParams params = {{"param1", "value1"}, {"param2", "value2"}};
    Test_Get_WithParams(httpbin_host, httpbin_port, httpbin_path_get, params);
    Test_Get_WithParams(postman_host, postman_port, postman_path_get, params);
}

TEST(HttpClientTestSuite, Get_WithParamsInPath) {
    // Case 2: Pass the path with paramteters
    int count = 0;
    ssgx::http_t::TypeParams params = {{"param1", "value1"}, {"param2", "value2"}};
    std::string params_str;
    for (const auto& [key, value] : params) {
        params_str += key;
        params_str += "=";
        params_str += value;
        params_str += (++count < params.size()) ? "&" : "";
    }

    Test_Get_PathOnly(httpbin_host, httpbin_port, httpbin_path_get + "?" + params_str);
    Test_Get_PathOnly(postman_host, postman_port, postman_path_get + "?" + params_str);
}

void Test_Get_WithHeadersAndParams(const std::string& host, int port, const std::string& path,
                                   const ssgx::http_t::TypeHeaders& headers, const ssgx::http_t::TypeParams& params) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Get(path, headers, params);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Get_WithHeadersAndParams) {
    // Case 1: Pass the path and paramteters separately.
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    ssgx::http_t::TypeParams params = {{"param1", "value1"}, {"param2", "value2"}};
    Test_Get_WithHeadersAndParams(httpbin_host, httpbin_port, httpbin_path_get, headers, params);
    Test_Get_WithHeadersAndParams(postman_host, postman_port, postman_path_get, headers, params);
}

TEST(HttpClientTestSuite, Get_WithHeadersAndParamsInPath) {
    // Case 2: Pass the path with paramteters
    int count = 0;
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    ssgx::http_t::TypeParams params = {{"param1", "value1"}, {"param2", "value2"}};
    std::string params_str;
    for (const auto& [key, value] : params) {
        params_str += key;
        params_str += "=";
        params_str += value;
        params_str += (++count < params.size()) ? "&" : "";
    }
    Test_Get_WithHeaders(httpbin_host, httpbin_port, httpbin_path_get + "?" + params_str, headers);
    Test_Get_WithHeaders(postman_host, postman_port, postman_path_get + "?" + params_str, headers);
}

void Test_Post_PathOnly(const std::string& host, int port, const std::string& path) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_PathOnly) {
    Test_Post_PathOnly(httpbin_host, httpbin_port, httpbin_path_post);
    Test_Post_PathOnly(postman_host, postman_port, postman_path_post);
}

void Test_Post_WithHeaders(const std::string& host, int port, const std::string& path,
                           const ssgx::http_t::TypeHeaders& headers) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path, headers);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_WithHeaders) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    Test_Post_WithHeaders(httpbin_host, httpbin_port, httpbin_path_post, headers);
    Test_Post_WithHeaders(postman_host, postman_port, postman_path_post, headers);
}

void Test_Post_WithBodyCharPointer(const std::string& host, int port, const std::string& path, const char* body) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path, body, std::strlen(body), "application/json");
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_WithBodyCharPointer) {
    const char* body = "{\"field1\": \"value1\",\"fiedl2\": 1000}";
    Test_Post_WithBodyCharPointer(httpbin_host, httpbin_port, httpbin_path_post, body);
    Test_Post_WithBodyCharPointer(postman_host, postman_port, postman_path_post, body);
}

void Test_Post_WithHeadersAndBodyCharPointer(const std::string& host, int port, const std::string& path,
                                             const ssgx::http_t::TypeHeaders& headers, const char* body) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path, headers, body, std::strlen(body), "application/json");
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_WithHeadersAndBodyCharPointer) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    const char* body = "{\"field1\": \"value1\",\"fiedl2\": 1000}";
    Test_Post_WithHeadersAndBodyCharPointer(httpbin_host, httpbin_port, httpbin_path_post, headers, body);
    Test_Post_WithHeadersAndBodyCharPointer(postman_host, postman_port, postman_path_post, headers, body);
}

void Test_Post_WithBodyString(const std::string& host, int port, const std::string& path, const std::string& body) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path, body, "application/json");
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_WithBodyString) {
    const std::string body = "{\"field1\": \"value1\",\"fiedl2\": 1000}";
    Test_Post_WithBodyString(httpbin_host, httpbin_port, httpbin_path_post, body);
    Test_Post_WithBodyString(postman_host, postman_port, postman_path_post, body);
}

void Test_Post_WithHeadersAndBodyString(const std::string& host, int port, const std::string& path,
                                        const ssgx::http_t::TypeHeaders& headers, const std::string& body) {
    ssgx::http_t::Client client(host, port);
    ssgx::http_t::Result result = client.Post(path, headers, body, "application/json");
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);
}
TEST(HttpClientTestSuite, Post_WithHeadersAndBodyString) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    const std::string body = "{\"field1\": \"value1\",\"fiedl2\": 1000}";
    Test_Post_WithHeadersAndBodyString(httpbin_host, httpbin_port, httpbin_path_post, headers, body);
    Test_Post_WithHeadersAndBodyString(postman_host, postman_port, postman_path_post, headers, body);
}

TEST(HttpClientTestSuite, Parse_GetResponseBody) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    ssgx::http_t::TypeParams params = {{"param1", "value1"}, {"param2", "value2"}};

    ssgx::http_t::Client client(postman_host);
    ssgx::http_t::Result result = client.Get(postman_path_get, headers, params);
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);

    nlohmann::json root_json = nlohmann::json::parse(result->Body());
    nlohmann::json args_json = root_json["args"];
    ASSERT_TRUE(args_json["param1"] == "value1");
    ASSERT_TRUE(args_json["param2"] == "value2");
    nlohmann::json headers_json = root_json["headers"];
    ASSERT_TRUE(headers_json["header1"] == "value1");
    ASSERT_TRUE(headers_json["header2"] == "value2");
}

TEST(HttpClientTestSuite, Parse_PostResponseBody) {
    ssgx::http_t::TypeHeaders headers = {{"header1", "value1"}, {"header2", "value2"}};
    const std::string body = "{\"field1\": \"value1\",\"fiedl2\": 1000}";

    ssgx::http_t::Client client(postman_host);
    ssgx::http_t::Result result = client.Post(postman_path_post, headers, body, "application/json");
    ASSERT_TRUE(result);
    ssgx::utils_t::Printf("%d\n", result->StatusCode());
    ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
    ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::OK200);

    nlohmann::json root_json = nlohmann::json::parse(result->Body());
    nlohmann::json headers_json = root_json["headers"];
    ASSERT_TRUE(headers_json["header1"] == "value1");
    ASSERT_TRUE(headers_json["header2"] == "value2");
    nlohmann::json json_json = root_json["json"];
    ASSERT_TRUE(json_json["field1"] == "value1");
    ASSERT_TRUE(json_json["fiedl2"] == 1000);
}

TEST(HttpClientTestSuite, ErrorHandling_InvalidUrl) {
    const std::string no_scheme_host = "127.0.0.1"; // No scheme
    ssgx::http_t::Client client(no_scheme_host);
    ssgx::http_t::Result result = client.Get(httpbin_path_get);
    ASSERT_TRUE(!result);
    ASSERT_EQ(result.error().code(), ssgx::http_t::ErrorCode::InvalidUrl);
    ssgx::utils_t::Printf("\n%s\n", result.error().message().c_str());

    const std::string invalid_scheme_host = "ftp://127.0.0.1"; // No http/https scheme
    client = ssgx::http_t::Client(invalid_scheme_host);
    result = client.Get(httpbin_path_get);
    ASSERT_TRUE(!result);
    ASSERT_EQ(result.error().code(), ssgx::http_t::ErrorCode::InvalidUrl);
    ssgx::utils_t::Printf("%s\n", result.error().message().c_str());
}

TEST(HttpClientTestSuite, ErrorHandling_InvalidHost) {
    const std::string invalid_host = "https://www.aaaa.com";
    ssgx::http_t::Client client(invalid_host);
    ssgx::http_t::Result result = client.Get(httpbin_path_get);
    ASSERT_TRUE(!result);
    ASSERT_EQ(result.error().code(), ssgx::http_t::ErrorCode::ConnectFailed);
    ssgx::utils_t::Printf("\n%s\n", result.error().message().c_str());
}

TEST(HttpClientTestSuite, ErrorHandling_InvalidPath) {
    const std::string invalid_path = "Get/invalid";
    ssgx::http_t::Client client(httpbin_host);
    ssgx::http_t::Result result = client.Get(invalid_path);
    ASSERT_TRUE(!result);
    ASSERT_EQ(result.error().code(), ssgx::http_t::ErrorCode::ConnectFailed);
    ssgx::utils_t::Printf("\n%s\n", result.error().message().c_str());
}

TEST(HttpClientTestSuite, ErrorHandling_InvalidCA) {
    // Use a wrong ca certificates chain to verify postman https certificate
    // We will get an error code in this case.
    ssgx::http_t::Client http(postman_host, postman_port, binance_ca);
    ssgx::http_t::Result result = http.Get(postman_path_get);
    ASSERT_TRUE(!result);
    ASSERT_EQ(result.error().code(), ssgx::http_t::ErrorCode::VerifyCertFailed);
    ssgx::utils_t::Printf("\nError: %s, internal error: %s\n", result.error().message().c_str(),
                          result.error().internal_error().c_str());
}

TEST(HttpClientTestSuite, ErrorHandling_InvalidTimeStamp) {
    ssgx::http_t::TypeHeaders headers = {{"X-MBX-APIKEY", "{Your-API-Key}"}};
    ssgx::http_t::TypeParams params = {{"omitZeroBalances", "true"},
                                   {"recvWindow", "60000"},
                                   {"timestamp", "1731565108329"},
                                   {"signature", "65cfe3fb183daf8937a881f646e03c3d03a4be7ed4600011b8ce36d59a3f2535"}};

    ssgx::http_t::Client http(binance_host, binance_port, binance_ca);
    if (auto result = http.Get(binance_account_path, headers, params)) {
        ssgx::utils_t::Printf("%d\n", result->StatusCode());
        ssgx::utils_t::Printf("%s\n\n", result->Body().c_str());
        ASSERT_TRUE(result->StatusCode() == ssgx::http_t::HttpStatusCode::BadRequest400);
        ASSERT_TRUE(result.error().code() == ssgx::http_t::ErrorCode::Success);
    } else {
        ssgx::utils_t::Printf("\nFailed to call GET method! error: %s, internal error: %s\n",
                              result.error().message().c_str(), result.error().internal_error().c_str());
        ASSERT_FALSE(result.error().code() == ssgx::http_t::ErrorCode::Success);
    }
}
