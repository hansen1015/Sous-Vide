const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Sous Vide Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; background-color: #f4f4f4; }
    .card { background: white; padding: 20px; margin: 10px auto; width: 90%; max-width: 600px; border-radius: 10px; box-shadow: 0 4px 8px 0 rgba(0,0,0,0.2); }
    h2 { color: #333; }
    .value { font-size: 2.5rem; color: #e67e22; font-weight: bold; }
    .label { font-size: 1.2rem; color: #666; }
    .btn { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 5px; }
    .btn-stop { background-color: #f44336; }
    .btn-preset { background-color: #008CBA; padding: 10px 20px; font-size: 14px; }
    input[type=number] { padding: 10px; font-size: 16px; width: 80px; }
  </style>
</head>
<body>
  <div class="card">
    <h2>ESP32 Sous Vide</h2>
    <p><span class="label">Current Temp:</span> <span id="temp" class="value">--</span> &deg;C</p>
    <p><span class="label">Target:</span> <span id="setpoint" class="value">--</span> &deg;C</p>
    <p><span class="label">Status:</span> <span id="status" style="font-weight:bold; color:green;">--</span></p>
    
    <div style="margin: 20px;">
      <canvas id="tempChart"></canvas>
    </div>

    <div>
      <input type="number" id="targetInput" value="55" step="0.5">
      <button class="btn" onclick="setTarget()">Set Temp</button>
    </div>
    
    <div style="margin-top: 20px;">
      <button class="btn" onclick="toggleRun(true)">START</button>
      <button class="btn btn-stop" onclick="toggleRun(false)">STOP</button>
    </div>

    <div style="margin-top: 20px;">
      <p>Presets:</p>
      <button class="btn btn-preset" onclick="setPreset(54)">Steak (54)</button>
      <button class="btn btn-preset" onclick="setPreset(63)">Eggs (63)</button>
      <button class="btn btn-preset" onclick="setPreset(60)">Chicken (60)</button>
      <button class="btn btn-preset" onclick="setPreset(85)">Veg (85)</button>
    </div>
    
    <div style="margin-top: 30px;">
        <a href="/update" class="btn" style="background-color: #555;">Update Firmware</a>
    </div>

  </div>

<script>
  var ctx = document.getElementById('tempChart').getContext('2d');
  var chart = new Chart(ctx, {
      type: 'line',
      data: {
          labels: [],
          datasets: [{ label: 'Temperature', borderColor: '#e67e22', data: [] }]
      },
      options: { scales: { x: { display: false } } }
  });

  function updateData() {
    fetch('/status').then(response => response.json()).then(data => {
      document.getElementById('temp').innerText = data.current.toFixed(1);
      document.getElementById('setpoint').innerText = data.target.toFixed(1);
      document.getElementById('status').innerText = data.running ? "HEATING" : "IDLE";
      if(data.error) document.getElementById('status').innerText = "ERROR: " + data.error;

      // Update Chart
      var now = new Date();
      if(chart.data.labels.length > 50) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
      }
      chart.data.labels.push(now.getSeconds());
      chart.data.datasets[0].data.push(data.current);
      chart.update();
    });
  }

  function setTarget() {
    var val = document.getElementById('targetInput').value;
    fetch('/set?target=' + val);
  }

  function setPreset(val) {
    document.getElementById('targetInput').value = val;
    setTarget();
  }

  function toggleRun(state) {
    fetch('/set?run=' + (state ? 1 : 0));
  }

  setInterval(updateData, 2000); // 2s polling
</script>
</body>
</html>
)rawliteral";
