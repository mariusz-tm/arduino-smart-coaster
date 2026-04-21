#pragma once

const char WIFI_FORM[] = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <title>Smart-Coaster WIFI Login</title>
        <style>
            body {
                margin: 0;
                padding: 0;
                background-color: #1C1919;
            }
            main {
                height: 90vh;
                align-content: center;
                text-align:center;
                font-family: Arial;
                color: #FD5E53;
            }

            h1, p {
                margin: 0;
            }
            input {
                width:200px;
                margin:5px;
                padding:8px;
            }
            .valid {
                color:red;
            }
        </style>
    </head>

    <body>
        <main>
            <h1>WIFI Setup</h1>
            <p style="margin-bottom: 10px; font-style: italic;">Enter WIFI credentials for the smartcoaster</p>

            <p class='valid'></p>

            <form action="/save">
                <input name="ssid" required placeholder="SSID"><br>
                <input name="pass" type="password" required placeholder="Password"><br>
                <input type="submit" value="Connect">
            </form>
        </main>
    </body>
</html>
)rawliteral";