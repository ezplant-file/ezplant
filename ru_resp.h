const char en_resp_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>OK</title>
<style>
body {
    margin: 0;
    padding: 0;
    overflow-x: hidden;
    min-width: 320px;
    background: #eee;
    font-family: Lato,'Helvetica Neue',Arial,Helvetica,sans-serif;
    font-size: 14px;
    line-height: 1.4285em;
    color: rgba(0,0,0,.87);
    font-smoothing: antialiased;
}
</style>
</head>
<body>
<h1>WiFi settings saved</h1>
<p> Device will try to connect with these credentials.</p>
<p> WiFi settings can be updated in case they are incorrect or WiFi access point is changed in "Menu" -> "WiFi" -> "Change" </p>
</body>
)rawliteral";
