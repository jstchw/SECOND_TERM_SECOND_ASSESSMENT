#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"EOF(
<html>
    <head>
        <title>ESP8266 - Control Panel</title>
        <style>
            body {
                width: 100wh;
                height: 90vh;
                color: #fff;
                background: linear-gradient(-45deg, #990d81, #E73C7E, #23A6D5, #23D5AB, #74ef70);
                background-size: 400% 400%;
                -webkit-animation: Gradient 10s ease infinite;
                -moz-animation: Gradient 10s ease infinite;
                animation: Gradient 10s ease infinite;
                font-family: 'Trebuchet MS', Sans-serif;
                font-size: 1.4em;
                background-attachment: fixed;
            }

            @keyframes Gradient {
                0% {
                    background-position: 0% 50%
                }
                50% {
                    background-position: 100% 50%
                }
                100% {
                    background-position: 0% 50%
                }
            }

            .box {
                margin: auto;
                text-align: center;
                /*border: 1px solid black;*/
                border-radius: 10px;
                width: 30%;
                padding: 1%;
                background: rgba(0,0,0,.7);
                box-shadow: 2px 3px 4px rgba(0,0,0,.5);
            }

            #content {
               margin-top: 1%;
            }

            .button {
                margin: 3%;
            }

        </style>
    </head>
    <body>
        <div class=box id=title>Control Panel</div>
        <div class=box id=content>
            <form action=/ method="POST">
                <div><input type=radio name="traffic" value="a">Traffic Lights Equal Priority</div>
                <div><input type=radio name="traffic" value="b">Traffic Lights Set 1 Priority</div>
                <div><input type=radio name="traffic" value="c">Traffic Lights Set 2 Priority</div>
                <div class="button"><input type=submit name="submit" value="Proceed"></div>
            </form>
        </div>
    </body>
</html>
)EOF";

static const char SERVER_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICuTCCAiKgAwIBAgIUMBoiYsLZx+NVBeJCHu1BX7bX1Y4wDQYJKoZIhvcNAQEL
BQAwZjELMAkGA1UEBhMCVUsxFjAUBgNVBAgMDUVhc3QgTWlkbGFuZHMxEjAQBgNV
BAcMCUxlaWNlc3RlcjEVMBMGA1UECgwMQXJ0ZW0gQm9icm92MRQwEgYDVQQDDAsx
OTIuMTY4LjQuMTAeFw0yMTAzMjMxMzExMzdaFw0yNTAzMjMxMzExMzdaMGYxCzAJ
BgNVBAYTAlVLMRYwFAYDVQQIDA1FYXN0IE1pZGxhbmRzMRIwEAYDVQQHDAlMZWlj
ZXN0ZXIxFTATBgNVBAoMDEFydGVtIEJvYnJvdjEUMBIGA1UEAwwLMTkyLjE2OC40
LjEwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALzdxGR1+fPUTmxtFBoCu3lU
nVtiWElmDR8wdrRVeRTr5DcRpExvcgy131nFTSgUTJK7lKuVZgsVdRrMYC/ROlVV
5EgXTiqkBl89I2o/qzoVXINSaagC8qLB4FMjHyCSGFi3yT0BKDiXPln8X7NqEmBS
KtAqDy4yGx+GHvbU1IQLAgMBAAGjZDBiMB0GA1UdDgQWBBRPXeUPKK0ixCWwg/Ik
s99tGxrqUDAfBgNVHSMEGDAWgBRPXeUPKK0ixCWwg/Iks99tGxrqUDAPBgNVHRMB
Af8EBTADAQH/MA8GA1UdEQQIMAaHBMCoBAEwDQYJKoZIhvcNAQELBQADgYEAe9GR
vESG2+mgZZ6QAKWkHlcGG+F9wRAEeckwMn7I/mrRZZzOOjYp3wMesRZvaId7kj0u
WjDqsQ7U10yu77zN0/PhkwgGlwDGEsDLHqVqMdKLsJw/WLKNeGd+BPP1e1BjYlaO
nWCeGjHefw4HCI8yBAyTrfOciIKX5ewZHvSQ/vM=
-----END CERTIFICATE-----
)EOF";

static const char SERVER_KEY[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBALzdxGR1+fPUTmxt
FBoCu3lUnVtiWElmDR8wdrRVeRTr5DcRpExvcgy131nFTSgUTJK7lKuVZgsVdRrM
YC/ROlVV5EgXTiqkBl89I2o/qzoVXINSaagC8qLB4FMjHyCSGFi3yT0BKDiXPln8
X7NqEmBSKtAqDy4yGx+GHvbU1IQLAgMBAAECgYA44am2f/mpv/VmnmuBcfU4k41Y
6oOgh4XIMPZvVZbEAE+ZbkzHYIE+5r+/77ZPLd3qC2i5TKORDwNX8Nm/TQp+CHPO
iAVcG1bzu2FYgfZFNMyTL92nlgfrHc0ybJey/mNBIuQpq872OHeDZprZ/7GtjMge
LehGWczp9Ozqgoxk0QJBAOTqoL0Q7NmI9CdFCf6AkAIOengFM7c11Qc7RrvpG1zo
kfqoQDt+w0arqWv1Nl1PI6Oqa4DgdqvVt2E1hUE5o7cCQQDTNhtOQaZEeJsElCnk
yO8WXlnU2za+qEU2I92BuEuiGswUm2PbeBgsGubAOELcGo9sdoyviCdtiLlJN1/J
7+pNAkBZ2Umry8fa/ikufKeWj+l2FS62ivwobeWmR/rUaINH7TTW2FEVyQRmaZw+
9mkXKKfJZmt5kyChA2pNqk8aMcBzAkAOeTe3ziHhsU4WKz0Ps4FGzCJlxrvips9P
XXfRatqr+iAgDWHzaTBm3VUQuQpD4QIREbKkWLUy+VqRHN1QjkNFAkEA0rpRCriR
FIAxSquxzI1H30Bz/5/9kxI3CSM3RyoseOwhZfteoaJ7Ct+xcIpd4t5XyGEQ7EW/
UDM2kEVKWmxwuw==
-----END PRIVATE KEY-----
)EOF";