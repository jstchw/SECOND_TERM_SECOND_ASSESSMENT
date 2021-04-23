#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"EOF(
<html>
    <head>
        <title>ESP8266 - Control Panel</title>
        <style>
            body {
                /* color: #fff; */
                font-family: 'Trebuchet MS', Sans-serif;
                font-size: 1.4em;
            }

            .box {
                margin: auto;
                text-align: center;
                /*border: 1px solid black;*/
                border-radius: 10px;
                width: 30%;
                padding: 1%;
                /* background: rgba(0,0,0,.7); */
                /* box-shadow: 2px 3px 4px rgba(0,0,0,.5); */
            }

            #content {
               margin-top: 1%;
            }

            .button {
                margin: 3%;
            }

            .bar {
                width: 30%;
            }

            .title-box {
                text-align: center;
                width: auto;
                padding: 1%;
                /* background: rgba(255, 255, 255, 0.7); */
                /* box-shadow: 2px 3px 4px rgba(0,0,0,.5); */
            }

            @media (pointer:none), (pointer:coarse) {
                body {
                    font-family: 'Courier New', monospace;
                    font-size: 4em;
                }

                .title-box {
                    background-color: rgb(0, 0, 0);
                    box-shadow: 0px 5px 0px rgba(0,0,0,.5);
                    color: #fff;
                    position:fixed;
                    top: 0;
                    left: 0;
                    width:100%;
                    z-index:100;
                }

                .box {
                    padding-top: 150px;
                    width: 100%;
                    font-style: oblique;
                    font-weight: 1000;
                }

                input[type="radio"] {
                    display: none;
                }

                input[type="radio"]:checked + label{
                    text-decoration: underline;
                }

                input[type="submit"] {
                    width: 15em;
                    height: 3em;
                    background-color: black;
                    box-shadow: 3px 4px 5px rgba(0,0,0,.5);
                    color: white;
                    font-size: 50px;
                    font-family: 'Courier New', monospace;
                }

                
            }

        </style>
    </head>
    <body>
        <div class=title-box>Control Panel</div>

        <div class=box id=content>
            <form action=/ method="POST">
                <div>- Traffic Lights -</div><hr class = bar>
                <label><div><input type=radio name="traffic" value="traffic_d" id="off1"><label for="off1">Off</label></div></label>
                <label><div><input type=radio name="traffic" value="traffic_a" id="eq"><label for="eq">Equal Priority</label></div></label>
                <label><div><input type=radio name="traffic" value="traffic_b" id="p1"><label for="p1">Set 1 Priority</label></div></label>
                <label><div><input type=radio name="traffic" value="traffic_c" id="p2"><label for="p2">Set 2 Priority</label></div></label>
                <br>
                <label><div class="button"><input type=submit name="submit" value="Apply"></div></label>
            </form>
            <form action=/ method="POST">
                <div>- Tri-color LED -</div><hr class = bar>
                <label><div><input type=radio name="tri" value="tri_a" id="off2"><label for="off2">Off</label></div></label>
                <label><div><input type=radio name="tri" value="tri_b" id="hazard"><label for="hazard">Hazard</label></div></label>
                <label><div><input type=radio name="tri" value="tri_c" id="emergency"><label for="emergency">Emergency</label></div></label>
                <label><div><input type=radio name="tri" value="tri_d" id="doctor"><label for="doctor">Doctor</label></div></label>
                <label><div><input type=radio name="tri" value="tri_e" id="police"><label for="police">Police</label></div></label>
                <br>
                <label><div class="button"><input type=submit name="submit" value="Apply"></div></label>
            </form>
            <form action=/ method="POST">
            <div>- Fruit Machine -</div><hr class = bar>
                <label><div><input type=radio name="fruit" value="fruit_a" id="off3"><label for="off3">Off</label></div></label>
                <label><div><input type=radio name="fruit" value="fruit_b" id="on3"><label for="on3">On</label></div></label>
                <br>
                <label><div class="button"><input type=submit name="submit" value="Apply"></div></label>
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