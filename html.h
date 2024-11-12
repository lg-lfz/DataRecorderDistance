#pragma once

const char htmlContent[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>DataRecorder</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link rel="icon" href="data:," />
    <style>
      html {
        font-family: Arial, Helvetica, sans-serif;
        text-align: center;
      }
      h1 {
        font-size: 1.8rem;
        color: white;
      }
      h2 {
        font-size: 1.5rem;
        font-weight: bold;
        color: #143642;
      }
      .topnav {
        overflow: hidden;
        background-color: #143642;
      }
      body {
        margin: 0;
      }
      .content {
        padding: 30px;
        max-width: 600px;
        margin: 0 auto;
      }
      .card {
        background-color: #f8f7f9;
        box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, 0.5);
        padding-top: 10px;
        padding-bottom: 20px;
      }
      .button {
        padding: 15px 50px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #0f8b8d;
        border: none;
        border-radius: 5px;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
      }
      button .line1 {
        display: block;
      }
      button .line2 {
        display: block;
        font-size: 1rem;
      }
      /*.button:hover {background-color: #0f8b8d}*/
      .button:active {
        background-color: #0f8b8d;
        box-shadow: 2 2px #cdcdcd;
        transform: translateY(2px);
      }
      .button:disabled,
      button[disabled] {
        background-color: #0f8b8d52;
        box-shadow: 2 2px #cdcdcd;
        transform: translateY(2px);
        cursor: not-allowed;
        pointer-events: none; /* Ensures the button is not clickable */
      }
      .state {
        font-size: 1.5rem;
        color: #8c8c8c;
        font-weight: bold;
      }
      .toast {
        position: fixed;
        bottom: 20px; /* Adjust as needed */
        left: 50%;
        transform: translateX(-50%);
        color: #fff;
        padding: 10px 20px;
        border-radius: 5px;
        font-size: 20px;
        display: none; /* Initially hidden */
        z-index: 1000; /* Ensure it's above other content */
      }
    </style>
    <title>DataRecorder</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link rel="icon" href="data:," />
  </head>
  <body>
    <div class="toast" id="toast"></div>
    <div class="topnav">
      <h1>DataRecorder</h1>
    </div>
    <div class="content">
      <div class="card">
        <h2 class="pressure">Luftdruck: <span id="pressure">0.0</span> hPa</h2>
        <h2 class="altitude">H&ouml;he: <span id="altitude">0.0</span> m</h2>
        <h2 class="temperature">Temperatur: <span id="temperature">0.0</span> &deg;C</h2>
        <h2 class="gas">Gas: <span id="gas">0.0</span> mL</h2>
        <h2 class="date_time_ISO8601">
          DataTime: <span id="date_time_ISO8601">1970-01-01T00:00:00Z</span>
        </h2>
        <p class="state">state: <span id="state">no recording...</span></p>
        <p><button id="Startbutton" class="button">Start</button></p>
        <p>
          <button disabled id="Download" class="button">
            <span class="line1">Download File</span>
            <span id="file_info" class="line2">Filesize...</span>
          </button>
        </p>
        <p><button id="SetDateTime" class="button">Set DateTime</button></p>
        <p><button id="FormatFS" class="button">Format Filesystem</button></p>
      </div>
    </div>
    <script>
      var gateway = `ws://${window.location.hostname}/ws`;
      var websocket;
      window.addEventListener("load", onLoad);
      function initWebSocket() {
        console.log("Trying to open a WebSocket connection...");
        websocket = new WebSocket(gateway);
        websocket.onopen = onOpen;
        websocket.onclose = onClose;
        websocket.onmessage = onMessage;
      }
      function onOpen(event) {
        console.log("Connection opened");
        const data = {
          action: "request",
        };
        websocket.send(JSON.stringify(data));
      }
      function onClose(event) {
        console.log("Connection closed");
        setTimeout(initWebSocket, 2000);
      }
      function setRecordingState() {
        document.getElementById("Startbutton").innerHTML = "Stop";
        document.getElementById("state").innerHTML = "RECORDING!";
      }
      function setNoRecordingState() {
        document.getElementById("Startbutton").innerHTML = "Start";
        document.getElementById("state").innerHTML = "no recording...";
      }
      function onMessage(event) {
        const data = JSON.parse(event.data);
        if (data.state) {
          if (data.state == "start") {
            setRecordingState();
          } else if (data.state == "stop") {
            setNoRecordingState();
          } else if (data.state == "started") {
            setRecordingState();
          } else if (data.state == "stopped") {
            setNoRecordingState();
          }
        } else if (data.date_time) {
          showToast("Clock successfully updated!", 2000);
        } else if (data.altitude) {
          document.getElementById("temperature").innerHTML =
            data.temperature.toFixed(2);
          document.getElementById("altitude").innerHTML =
            data.altitude.toFixed(2);
          document.getElementById("pressure").innerHTML =
            data.pressure.toFixed(2);
          document.getElementById("gas").innerHTML =
            data.gas.toFixed(2);
          document.getElementById("date_time_ISO8601").innerHTML =
            data.date_time_ISO8601;
        } else if (data.freespace) {
          let info_text =
            "File: " +
            data.filename +
            " " +
            data.filesize +
            "bytes / " +
            data.freespace +
            " free.";
          document.getElementById("file_info").textContent = info_text;
          const DownloadButton = document.getElementById("Download");
          DownloadButton.disabled = false;
        }
      }
      function showToast(message, time, backgroundColor = "black") {
        var toast = document.getElementById("toast");
        toast.textContent = message;
        toast.style.backgroundColor = backgroundColor;
        toast.style.display = "block";
        if (time > 0) {
          setTimeout(function () {
            toast.style.display = "none";
          }, time); // Adjust time as needed (1-2 sec?)
        }
      }
      function onLoad(event) {
        initWebSocket();
        initButton();
      }
      function initButton() {
        document
          .getElementById("Startbutton")
          .addEventListener("click", toggle);
        document
          .getElementById("SetDateTime")
          .addEventListener("click", set_date_time);
        document
          .getElementById("FormatFS")
          .addEventListener("click", format_fs);
        document.getElementById("Download").addEventListener("click", download);
      }
      function toggle() {
        const data = {
          action: "toggle",
        };
        websocket.send(JSON.stringify(data));
      }
      function set_date_time() {
        let now = new Date();
        const data = {
          date_time: now.toISOString(),
        };
        websocket.send(JSON.stringify(data));
      }
      function download() {
        window.location.href = "download";
      }
      function format_fs() {
        let now = new Date();
        const data = {
          action: "format",
        };
        showToast(
          "Filesystem would be formated, you have to reconnect...",
          -1,
          "red"
        );
        setNoRecordingState();
        websocket.send(JSON.stringify(data));
        setTimeout(function () {
          // Reload from the server
          window.location.href = window.location.href;
        }, 5000);
      }
    </script>
  </body>
</html>
)rawliteral";
