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
                               "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
                               "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
                               "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
                               "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
                               "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
                               "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
                               "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
                               "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
                               "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
                               "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
                               "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
                               "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
                               "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
                               "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
                               "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
                               "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
                               "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
                               "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
                               "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
                               "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
                               "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
                               "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
                               "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
                               "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
                               "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
                               "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
                               "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
                               "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
                               "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN CERTIFICATE-----\n"
                               "MIIEVzCCAj+gAwIBAgIRAIOPbGPOsTmMYgZigxXJ/d4wDQYJKoZIhvcNAQELBQAw\n"
                               "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
                               "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\n"
                               "WhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
                               "RW5jcnlwdDELMAkGA1UEAxMCRTUwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQNCzqK\n"
                               "a2GOtu/cX1jnxkJFVKtj9mZhSAouWXW0gQI3ULc/FnncmOyhKJdyIBwsz9V8UiBO\n"
                               "VHhbhBRrwJCuhezAUUE8Wod/Bk3U/mDR+mwt4X2VEIiiCFQPmRpM5uoKrNijgfgw\n"
                               "gfUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcD\n"
                               "ATASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSfK1/PPCFPnQS37SssxMZw\n"
                               "i9LXDTAfBgNVHSMEGDAWgBR5tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcB\n"
                               "AQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0g\n"
                               "BAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVu\n"
                               "Y3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAH3KdNEVCQdqk0LKyuNImTKdRJY1C\n"
                               "2uw2SJajuhqkyGPY8C+zzsufZ+mgnhnq1A2KVQOSykOEnUbx1cy637rBAihx97r+\n"
                               "bcwbZM6sTDIaEriR/PLk6LKs9Be0uoVxgOKDcpG9svD33J+G9Lcfv1K9luDmSTgG\n"
                               "6XNFIN5vfI5gs/lMPyojEMdIzK9blcl2/1vKxO8WGCcjvsQ1nJ/Pwt8LQZBfOFyV\n"
                               "XP8ubAp/au3dc4EKWG9MO5zcx1qT9+NXRGdVWxGvmBFRAajciMfXME1ZuGmk3/GO\n"
                               "koAM7ZkjZmleyokP1LGzmfJcUd9s7eeu1/9/eg5XlXd/55GtYjAM+C4DG5i7eaNq\n"
                               "cm2F+yxYIPt6cbbtYVNJCGfHWqHEQ4FYStUyFnv8sjyqU8ypgZaNJ9aVcWSICLOI\n"
                               "E1/Qv/7oKsnZCWJ926wU6RqG1OYPGOi1zuABhLw61cuPVDT28nQS/e6z95cJXq0e\n"
                               "K1BcaJ6fJZsmbjRgD5p3mvEf5vdQM7MCEvU0tHbsx2I5mHHJoABHb8KVBgWp/lcX\n"
                               "GWiWaeOyB7RP+OfDtvi2OsapxXiV7vNVs7fMlrRjY1joKaqmmycnBvAq14AEbtyL\n"
                               "sVfOS66B8apkeFX2NY4XPEYV4ZSCe8VHPrdrERk2wILG3T/EGmSIkCYVUMSnjmJd\n"
                               "VQD9F6Na/+zmXCc=\n"
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
