#include <WiFi.h>
#include <WebServer.h>
#include <PZEM004Tv30.h>

// ---------- WIFI ----------
const char* ssid = "Votre_WiFi";
const char* password = "MotDePasse";
// ---------- PZEM ----------
#define RXD2 16
#define TXD2 17
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);

// ---------- SERVER ----------
WebServer server(80);

// ---------- PAGE WEB AVEC GRAPHIQUES ----------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Power Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 1400px;
      margin: 0 auto;
    }
    header {
      background: rgba(255, 255, 255, 0.95);
      padding: 25px;
      border-radius: 15px;
      margin-bottom: 25px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      text-align: center;
    }
    h1 {
      color: #667eea;
      font-size: 32px;
      margin-bottom: 5px;
    }
    .subtitle {
      color: #666;
      font-size: 14px;
    }
    .status {
      display: inline-block;
      padding: 5px 15px;
      background: #10b981;
      color: white;
      border-radius: 20px;
      font-size: 12px;
      margin-top: 10px;
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 20px;
      margin-bottom: 25px;
    }
    .card {
      background: rgba(255, 255, 255, 0.95);
      padding: 25px;
      border-radius: 15px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      transition: transform 0.3s ease, box-shadow 0.3s ease;
      position: relative;
      overflow: hidden;
    }
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 12px 40px rgba(0, 0, 0, 0.15);
    }
    .card::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 4px;
      background: linear-gradient(90deg, #667eea, #764ba2);
    }
    .card-icon {
      width: 50px;
      height: 50px;
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 24px;
      margin-bottom: 15px;
      font-weight: bold;
    }
    .voltage-icon { background: #fef3c7; color: #f59e0b; }
    .current-icon { background: #dbeafe; color: #3b82f6; }
    .power-icon { background: #fce7f3; color: #ec4899; }
    .energy-icon { background: #d1fae5; color: #10b981; }
    .frequency-icon { background: #e0e7ff; color: #6366f1; }
    .pf-icon { background: #fce7f3; color: #ec4899; }
    .title {
      font-size: 14px;
      color: #666;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 10px;
      font-weight: 600;
    }
    .value {
      font-size: 36px;
      font-weight: bold;
      color: #1f2937;
      margin-bottom: 5px;
    }
    .unit {
      font-size: 18px;
      color: #9ca3af;
      font-weight: normal;
    }
    .description {
      font-size: 12px;
      color: #9ca3af;
      margin-top: 8px;
    }
    .chart-container {
      background: rgba(255, 255, 255, 0.95);
      padding: 25px;
      border-radius: 15px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      margin-bottom: 25px;
      position: relative;
    }
    .chart-container::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 4px;
      background: linear-gradient(90deg, #667eea, #764ba2);
    }
    .chart-title {
      font-size: 18px;
      font-weight: 600;
      color: #1f2937;
      margin-bottom: 20px;
      text-align: center;
    }
    .charts-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(500px, 1fr));
      gap: 25px;
      margin-bottom: 25px;
    }
    footer {
      background: rgba(255, 255, 255, 0.95);
      padding: 20px;
      border-radius: 15px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      text-align: center;
      color: #666;
      font-size: 13px;
    }
    .last-update {
      color: #9ca3af;
      margin-top: 5px;
    }
    @media (max-width: 768px) {
      h1 { font-size: 24px; }
      .value { font-size: 28px; }
      .grid { grid-template-columns: 1fr; }
      .charts-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
<div class="container">
  <header>
    <h1>Moniteur de Puissance ESP32</h1>
    <div class="subtitle">Surveillance en temps réel - PZEM-004T</div>
    <div class="status">En ligne</div>
  </header>

  <div class="grid">
    <div class="card">
      <div class="card-icon voltage-icon">V</div>
      <div class="title">TENSION</div>
      <div class="value"><span id="v">--</span> <span class="unit">V</span></div>
      <div class="description">Voltage du réseau électrique</div>
    </div>

    <div class="card">
      <div class="card-icon current-icon">A</div>
      <div class="title">COURANT</div>
      <div class="value"><span id="i">--</span> <span class="unit">A</span></div>
      <div class="description">Intensité consommée</div>
    </div>

    <div class="card">
      <div class="card-icon power-icon">W</div>
      <div class="title">PUISSANCE</div>
      <div class="value"><span id="p">--</span> <span class="unit">W</span></div>
      <div class="description">Puissance active instantanée</div>
    </div>

    <div class="card">
      <div class="card-icon energy-icon">kWh</div>
      <div class="title">ENERGIE</div>
      <div class="value"><span id="e">--</span> <span class="unit">kWh</span></div>
      <div class="description">Energie totale consommée</div>
    </div>

    <div class="card">
      <div class="card-icon frequency-icon">Hz</div>
      <div class="title">FREQUENCE</div>
      <div class="value"><span id="f">--</span> <span class="unit">Hz</span></div>
      <div class="description">Fréquence du réseau</div>
    </div>

    <div class="card">
      <div class="card-icon pf-icon">PF</div>
      <div class="title">FACTEUR DE PUISSANCE</div>
      <div class="value"><span id="pf">--</span></div>
      <div class="description">Efficacité énergétique (0-1)</div>
    </div>
  </div>

  <div class="charts-grid">
    <div class="chart-container">
      <div class="chart-title">📈 Tension (V) - Temps Réel</div>
      <canvas id="voltageChart"></canvas>
    </div>

    <div class="chart-container">
      <div class="chart-title">📊 Courant (A) - Temps Réel</div>
      <canvas id="currentChart"></canvas>
    </div>
  </div>

  <div class="chart-container">
    <div class="chart-title">⚡ Puissance (W) - Temps Réel</div>
    <canvas id="powerChart"></canvas>
  </div>

  <footer>
    <div>ESP32 Power Monitoring System</div>
    <div class="last-update">Dernière mise à jour: <span id="timestamp">--</span></div>
  </footer>
</div>

<script>
// Configuration des graphiques
const maxDataPoints = 30;
const timeLabels = [];
const voltageData = [];
const currentData = [];
const powerData = [];

// Initialisation des graphiques
const voltageChart = new Chart(document.getElementById('voltageChart'), {
  type: 'line',
  data: {
    labels: timeLabels,
    datasets: [{
      label: 'Tension (V)',
      data: voltageData,
      borderColor: '#f59e0b',
      backgroundColor: 'rgba(245, 158, 11, 0.1)',
      borderWidth: 2,
      tension: 0.4,
      fill: true,
      pointRadius: 3,
      pointHoverRadius: 5
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: true,
    aspectRatio: 2,
    plugins: {
      legend: {
        display: true,
        position: 'top'
      }
    },
    scales: {
      y: {
        beginAtZero: false,
        suggestedMin: 200,
        suggestedMax: 240,
        ticks: {
          callback: function(value) {
            return value + ' V';
          }
        }
      },
      x: {
        display: true,
        ticks: {
          maxRotation: 45,
          minRotation: 45
        }
      }
    },
    animation: {
      duration: 300
    }
  }
});

const currentChart = new Chart(document.getElementById('currentChart'), {
  type: 'line',
  data: {
    labels: timeLabels,
    datasets: [{
      label: 'Courant (A)',
      data: currentData,
      borderColor: '#3b82f6',
      backgroundColor: 'rgba(59, 130, 246, 0.1)',
      borderWidth: 2,
      tension: 0.4,
      fill: true,
      pointRadius: 3,
      pointHoverRadius: 5
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: true,
    aspectRatio: 2,
    plugins: {
      legend: {
        display: true,
        position: 'top'
      }
    },
    scales: {
      y: {
        beginAtZero: true,
        ticks: {
          callback: function(value) {
            return value + ' A';
          }
        }
      },
      x: {
        display: true,
        ticks: {
          maxRotation: 45,
          minRotation: 45
        }
      }
    },
    animation: {
      duration: 300
    }
  }
});

const powerChart = new Chart(document.getElementById('powerChart'), {
  type: 'line',
  data: {
    labels: timeLabels,
    datasets: [{
      label: 'Puissance (W)',
      data: powerData,
      borderColor: '#ec4899',
      backgroundColor: 'rgba(236, 72, 153, 0.1)',
      borderWidth: 2,
      tension: 0.4,
      fill: true,
      pointRadius: 3,
      pointHoverRadius: 5
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: true,
    aspectRatio: 2.5,
    plugins: {
      legend: {
        display: true,
        position: 'top'
      }
    },
    scales: {
      y: {
        beginAtZero: true,
        ticks: {
          callback: function(value) {
            return value + ' W';
          }
        }
      },
      x: {
        display: true,
        ticks: {
          maxRotation: 45,
          minRotation: 45
        }
      }
    },
    animation: {
      duration: 300
    }
  }
});

// Fonction pour mettre à jour les graphiques
function updateCharts(voltage, current, power) {
  const now = new Date();
  const timeString = now.toLocaleTimeString('fr-FR');
  
  // Ajouter les nouvelles données
  timeLabels.push(timeString);
  voltageData.push(voltage);
  currentData.push(current);
  powerData.push(power);
  
  // Limiter le nombre de points affichés
  if (timeLabels.length > maxDataPoints) {
    timeLabels.shift();
    voltageData.shift();
    currentData.shift();
    powerData.shift();
  }
  
  // Mettre à jour les graphiques
  voltageChart.update('none');
  currentChart.update('none');
  powerChart.update('none');
}

// Récupération des données
setInterval(() => {
  fetch("/data")
    .then(response => response.json())
    .then(data => {
      // Mettre à jour les valeurs numériques
      document.getElementById("v").innerHTML = data.voltage;
      document.getElementById("i").innerHTML = data.current;
      document.getElementById("p").innerHTML = data.power;
      document.getElementById("e").innerHTML = data.energy;
      document.getElementById("f").innerHTML = data.frequency;
      document.getElementById("pf").innerHTML = data.pf;
      
      // Mettre à jour les graphiques
      updateCharts(
        parseFloat(data.voltage),
        parseFloat(data.current),
        parseFloat(data.power)
      );
      
      const now = new Date();
      document.getElementById("timestamp").innerHTML = now.toLocaleTimeString('fr-FR');
    })
    .catch(err => console.error('Erreur:', err));
}, 2000);
</script>
</body>
</html>
)rawliteral";

// ---------- HANDLERS ----------
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleData() {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  
  String json = "{";
  json += "\"voltage\":" + String(voltage,1) + ",";
  json += "\"current\":" + String(current,2) + ",";
  json += "\"power\":" + String(power,1) + ",";
  json += "\"energy\":" + String(energy,3) + ",";
  json += "\"frequency\":" + String(frequency,1) + ",";
  json += "\"pf\":" + String(pf,2);
  json += "}";
  
  server.send(200, "application/json", json);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  Serial.println("\nDemarrage ESP32 Power Monitor...");
  
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connecte!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Ouvrez cette adresse dans votre navigateur");
  
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  
  Serial.println("Serveur web demarre!");
}

// ---------- LOOP ----------
void loop() {
  server.handleClient();
}
