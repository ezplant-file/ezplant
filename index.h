const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>WiFi connection</title>
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

.box{
        width: 70%;
        padding: 40px;
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%,-50%);
        background-color: #fff;
        /*color: white; */
        text-align: left;
        border-radius: 5px;
}

h1 {
        font-size: 20px;
        line-height: 50px;
        text-align: left;
        font-weight: 500;
}

input {
    margin: 0;
    max-width: 100%;
    -webkit-box-flex: 1;
    -ms-flex: 1 0 auto;
    flex: 1 0 auto;
    outline: 0;
    -webkit-tap-highlight-color: rgba(255,255,255,0);
    text-align: left;
    line-height: 1.21428571em;
    font-family: Lato,'Helvetica Neue',Arial,Helvetica,sans-serif;
    padding: 0.67857143em 1em;
    background: #fff;
    border: 1px solid rgba(34,36,38,.15);
    color: rgba(0,0,0,.87);
    border-radius: 0.28571429rem;
    -webkit-transition: border-color .1s ease,-webkit-box-shadow .1s ease;
    transition: border-color .1s ease,-webkit-box-shadow .1s ease;
    transition: box-shadow .1s ease,border-color .1s ease;
    transition: box-shadow .1s ease,border-color .1s ease,-webkit-box-shadow .1s ease;
    -webkit-box-shadow: none;
    box-shadow: none;
}

label {
    display: block;
    margin: 0 0 0.28571429rem 0;
    color: rgba(0,0,0,.87);
    font-size: .92857143em;
    font-weight: 700;
    text-transform: none;
}

input[type='submit'] {
        cursor: pointer;
        background-color: #2185d0;
        color: #fff;
        text-shadow: none;
        background-image: none;
}

input[type='submit']:hover{ background-color: #22a6b3; }

</style>
</head>
<body>
<form action="/get" class="box" id="my-form">
<h1>Введите данные точки доступа</h1>
<div class="part">
        <label>SSID</label>
<input name="ssid" type="text" placeholder="Имя точки доступа WiFi">
</div>
<div class="part">
        <label>Password</label>
<input type="password" name="password" placeholder="Пароль">
</div>
<input type="submit" value="Ввод">
</form></body></html>
)rawliteral";
