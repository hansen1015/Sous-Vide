const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <title>Sous Vide Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Fira+Code:wght@300;400;500;600&display=swap');
    
    * { 
      margin: 0; 
      padding: 0; 
      box-sizing: border-box; 
    }
    
    body { 
      font-family: 'Fira Code', 'Consolas', 'Monaco', monospace;
      background: #1e1e1e;
      color: #d4d4d4;
      min-height: 100vh;
      padding: 20px;
      line-height: 1.6;
    }
    
    .container {
      max-width: 1000px;
      margin: 0 auto;
    }
    
    h1 {
      text-align: center;
      color: #4ec9b0;
      font-size: 2rem;
      margin-bottom: 30px;
      font-weight: 500;
      letter-spacing: -0.5px;
    }
    
    h1::before {
      content: '// ';
      color: #6a9955;
    }
    
    .card {
      background: #252526;
      border: 1px solid #3e3e42;
      border-radius: 8px;
      padding: 25px;
      margin-bottom: 20px;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.4);
    }
    
    .error {
      background: #2d2d30;
      border: 1px solid #f48771;
      border-left: 4px solid #f48771;
      color: #f48771;
      padding: 15px;
      border-radius: 4px;
      margin: 20px 0;
      display: none;
      font-size: 0.9rem;
    }
    
    .error strong {
      display: block;
      margin-bottom: 5px;
      font-size: 1rem;
    }
    
    .error-code {
      font-family: 'Fira Code', monospace;
      font-weight: 600;
      font-size: 1rem;
      color: #ce9178;
    }
    
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 15px;
      margin-bottom: 25px;
    }
    
    .status-box {
      background: #2d2d30;
      border: 1px solid #3e3e42;
      border-radius: 6px;
      padding: 20px;
      text-align: center;
      transition: border-color 0.2s ease;
    }
    
    .status-box:hover {
      border-color: #569cd6;
    }
    
    .label {
      font-size: 0.75rem;
      color: #858585;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 10px;
      font-weight: 400;
    }
    
    .label::before {
      content: '> ';
      color: #569cd6;
    }
    
    .value {
      font-size: 2.2rem;
      font-weight: 600;
      color: #4ec9b0;
    }
    
    .chart-container {
      background: #2d2d30;
      border: 1px solid #3e3e42;
      border-radius: 6px;
      padding: 20px;
      margin: 20px 0;
    }
    
    .controls {
      display: flex;
      gap: 12px;
      align-items: center;
      justify-content: center;
      flex-wrap: wrap;
      margin: 20px 0;
    }
    
    input[type=number] {
      background: #3c3c3c;
      border: 1px solid #3e3e42;
      color: #d4d4d4;
      padding: 10px 15px;
      font-size: 1rem;
      border-radius: 4px;
      width: 100px;
      text-align: center;
      font-family: 'Fira Code', monospace;
      transition: border-color 0.2s ease;
    }
    
    input[type=number]:focus {
      outline: none;
      border-color: #569cd6;
      background: #2d2d30;
    }
    
    .btn {
      background: #0e639c;
      border: 1px solid #1177bb;
      color: #ffffff;
      padding: 10px 24px;
      font-size: 0.9rem;
      font-weight: 500;
      border-radius: 4px;
      cursor: pointer;
      transition: all 0.2s ease;
      font-family: 'Fira Code', monospace;
    }
    
    .btn:hover {
      background: #1177bb;
      border-color: #1c8ad6;
    }
    
    .btn:active {
      transform: translateY(1px);
    }
    
    .btn-stop {
      background: #f48771;
      border-color: #f48771;
      color: #1e1e1e;
    }
    
    .btn-stop:hover {
      background: #ff6b6b;
      border-color: #ff6b6b;
    }
    
    .btn-preset {
      background: #608b4e;
      border-color: #6a9955;
      font-size: 0.85rem;
      padding: 8px 16px;
    }
    
    .btn-preset:hover {
      background: #6a9955;
      border-color: #7ba757;
    }
    
    .preset-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 10px;
      margin: 20px 0;
    }
    
    .slider-container {
      margin: 25px 0;
      padding: 20px;
      background: #2d2d30;
      border: 1px solid #3e3e42;
      border-radius: 6px;
    }
    
    .slider-container .label {
      text-align: left;
      margin-bottom: 15px;
      font-size: 0.85rem;
    }
    
    input[type=range] {
      width: 100%;
      height: 6px;
      border-radius: 3px;
      background: #3c3c3c;
      outline: none;
      -webkit-appearance: none;
    }
    
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: #569cd6;
      cursor: pointer;
      border: 2px solid #1e1e1e;
      transition: all 0.2s ease;
    }
    
    input[type=range]::-webkit-slider-thumb:hover {
      background: #1c8ad6;
      transform: scale(1.1);
    }
    
    input[type=range]::-moz-range-thumb {
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: #569cd6;
      cursor: pointer;
      border: 2px solid #1e1e1e;
      transition: all 0.2s ease;
    }
    
    input[type=range]::-moz-range-thumb:hover {
      background: #1c8ad6;
      transform: scale(1.1);
    }
    
    .status-running {
      color: #4ec9b0;
    }
    
    .status-idle {
      color: #858585;
    }
    
    .status-error {
      color: #f48771;
    }
    
    a {
      color: #569cd6;
      text-decoration: none;
    }
    
    a:hover {
      text-decoration: underline;
    }
    
    .section-header {
      color: #6a9955;
      font-size: 0.9rem;
      margin: 25px 0 15px 0;
      text-align: center;
    }
    
    .section-header::before {
      content: '/* ';
    }
    
    .section-header::after {
      content: ' */';
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Sous Vide Controller</h1>
    
    <div class="card">
      <div id="errorDisplay" class="error">
        <strong>⚠ EMERGENCY STOP</strong>
        <span id="errorReason"></span><br>
        <span class="error-code" id="errorCode"></span>
      </div>
      
      <div class="status-grid">
        <div class="status-box">
          <div class="label">Avg Temp</div>
          <div class="value" id="temp">--</div>
        </div>
        <div class="status-box">
          <div class="label">Target Temp</div>
          <div class="value" id="setpoint">--</div>
        </div>
        <div class="status-box">
          <div class="label">PWM Output</div>
          <div class="value" id="pwmOut" style="color: #ce9178;">--</div>
        </div>
        <div class="status-box">
          <div class="label">Sensor 1</div>
          <div class="value" id="t1" style="font-size: 1.8rem;">--</div>
        </div>
        <div class="status-box">
          <div class="label">Sensor 2</div>
          <div class="value" id="t2" style="font-size: 1.8rem;">--</div>
        </div>
        <div class="status-box">
          <div class="label">Status</div>
          <div class="value" id="status" style="font-size: 1.4rem;">--</div>
        </div>
        <div class="status-box">
          <div class="label">Power Limit</div>
          <div class="value" id="powerValue" style="font-size: 1.8rem;">--</div>
        </div>
      </div>
      
      <div class="chart-container">
        <canvas id="tempChart"></canvas>
      </div>
      
      <div class="controls">
        <input type="number" id="targetInput" value="55" step="0.5" min="20" max="90">
        <button class="btn" onclick="setTarget()">Set Temp</button>
      </div>
      
      <div class="controls">
        <button class="btn" onclick="toggleRun(true)">▶ START</button>
        <button class="btn btn-stop" onclick="toggleRun(false)">■ STOP</button>
      </div>
      
      <div class="section-header">Quick Presets</div>
      <div class="preset-grid">
        <button class="btn btn-preset" onclick="setPreset(54)">🥩 Steak 54°C</button>
        <button class="btn btn-preset" onclick="setPreset(63)">🥚 Eggs 63°C</button>
        <button class="btn btn-preset" onclick="setPreset(60)">🍗 Chicken 60°C</button>
        <button class="btn btn-preset" onclick="setPreset(85)">🥦 Veg 85°C</button>
      </div>
      
      <div class="slider-container">
        <div class="label">Power Limit: <span id="powerSliderValue" style="color: #4ec9b0;">100</span>%</div>
        <input type="range" id="powerLimit" min="0" max="100" value="100" oninput="updatePowerLabel()" onchange="setPower()">
      </div>
      
      <div style="text-align: center; margin-top: 30px;">
        <a href="/update" class="btn" style="display: inline-block; background: #3c3c3c; border-color: #3e3e42;">🔧 Firmware Update</a>
      </div>
    </div>
  </div>

<script>
  var ctx = document.getElementById('tempChart').getContext('2d');
  var chart = new Chart(ctx, {
      type: 'line',
      data: {
          labels: [],
          datasets: [{
            label: 'Temperature (°C)',
            borderColor: '#4ec9b0',
            backgroundColor: 'rgba(78, 201, 176, 0.1)',
            data: [],
            yAxisID: 'y', // Left Axis
            tension: 0.3,
            fill: true,
            borderWidth: 2,
            pointRadius: 0,
            pointHoverRadius: 4
          }, {
            label: 'PWM Output (%)',
            borderColor: '#ff9f43',
            backgroundColor: 'rgba(255, 159, 67, 0)',
            data: [],
            yAxisID: 'y1', // Right Axis
            tension: 0.1,
            fill: false,
            borderWidth: 1,
            pointRadius: 0
          }]
      },
      options: {
        responsive: true,
        stacked: false,
        plugins: {
          legend: {
            labels: { 
              color: '#d4d4d4',
              font: { family: "'Fira Code', monospace", size: 11 }
            }
          }
        },
        scales: {
          x: { display: false },
          y: {
            type: 'linear',
            display: true,
            position: 'left',
            ticks: { color: '#858585', font: { family: "'Fira Code', monospace" } },
            grid: { color: '#3e3e42' }
          },
          y1: {
            type: 'linear',
            display: true,
            position: 'right',
            min: 0,
            max: 100,
            grid: { drawOnChartArea: false }, // Avoid grid clutter
            ticks: { color: '#ff9f43', font: { family: "'Fira Code', monospace" } }
          }
        }
      }
  });

  function updateData() {
    fetch('/status').then(response => response.json()).then(data => {
      document.getElementById('temp').innerText = data.current.toFixed(1) + '°C';
      document.getElementById('setpoint').innerText = data.target.toFixed(1) + '°C';
      
      // New Fields
      if(document.getElementById('t1')) document.getElementById('t1').innerText = data.t1.toFixed(1) + '°C';
      if(document.getElementById('t2')) document.getElementById('t2').innerText = data.t2.toFixed(1) + '°C';
      if(document.getElementById('pwmOut')) document.getElementById('pwmOut').innerText = data.pwm.toFixed(0) + '%';

      document.getElementById('powerValue').innerText = data.powerLimit + '%';
      document.getElementById('powerLimit').value = data.powerLimit;
      document.getElementById('powerSliderValue').innerText = data.powerLimit;
      
      const statusEl = document.getElementById('status');
      const errorDisplay = document.getElementById('errorDisplay');
      
      if(data.error) {
        statusEl.innerText = '⚠ ERROR';
        statusEl.className = 'value status-error';
        errorDisplay.style.display = 'block';
        document.getElementById('errorReason').innerText = data.error;
        document.getElementById('errorCode').innerText = data.errorCode ? 'Error Code: E' + String(data.errorCode).padStart(2, '0') : '';
      } else {
        errorDisplay.style.display = 'none';
        if(data.running) {
          statusEl.innerText = '⚡ HEATING';
          statusEl.className = 'value status-running';
        } else {
          statusEl.innerText = '⏸ IDLE';
          statusEl.className = 'value status-idle';
        }
      }

      // Update Chart
      if(chart.data.labels.length > 50) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
          if(chart.data.datasets[1]) chart.data.datasets[1].data.shift();
      }
      chart.data.labels.push(new Date().toLocaleTimeString());
      chart.data.datasets[0].data.push(data.current);
      if(chart.data.datasets[1]) chart.data.datasets[1].data.push(data.pwm);
      chart.update('none');
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
  
  function updatePowerLabel() {
    var val = document.getElementById('powerLimit').value;
    document.getElementById('powerSliderValue').innerText = val;
  }
  
  function setPower() {
    var val = document.getElementById('powerLimit').value;
    fetch('/set?limit=' + val);
  }

  setInterval(updateData, 2000); // 2s polling
  updateData(); // Initial load
</script>
</body>
</html>
)rawliteral";
