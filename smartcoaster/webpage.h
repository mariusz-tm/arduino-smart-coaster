// webpage.h
#pragma once

const char MAIN_page[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Smart Coaster Project</title>
  <style>
    main{
      display: flex;
      flex-direction: column;
      align-items: center; 
    }
    body{
      background-color: #1C1919;
      color: white;
      font-family: Verdana, Geneva, Tahoma, sans-serif;
    }

    section{
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 15px;
      margin-top: 50px;
    }
    h2{
      background-color: #FD5E53;
      padding: 10px 15px;
      border-radius: 5px;
      display: inline-block;
      font-size: 2em;
    }
    h3{
      font-size: 0.8em;
      font-weight: normal;
      color: rgb(165, 159, 159)
    }
    a {
      font-size: 1em;
      font-family: Verdana, Geneva, Tahoma, sans-serif;
      color: white;
      text-decoration-color: #FD5E53;
      text-decoration-thickness: 1px;
      margin: 0;
    }
    button{
      padding: 5px;
      border: 5px solid #FD5E53;
      border-radius: 25px;
      background-color: #f78c84;
      font-size: 1em;
      color: white;
    }
    button:active{
      border: 5px solid #481916;
      background-color: #8a2c26;
      color: gray;
    }
    .mainBox{
      width: 31.25%;
      height: 150px;
      border: 3px solid #FD5E53;
      border-radius: 10px;
      background-color: #161111;
      position: relative;
      box-sizing: border-box;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      text-align: left;
      margin-top: 10px;
      min-height: 40vh;
  </style>
</head>

<body>
  <main>
    <section>
      <h2>Smart Coaster</h2>
      <h3>
        <em>
          A burn free way of testing your drink's temperature
          <br>
          Collaborative project completed by ATU Students
          <br>
          <a href="https://github.com/mariusz-tm/arduino-smart-coaster">Github Repository found here!</a>
        </em>
      </h3>
    </section>

    <section class="mainBox">
      <p>
        Your Desired Temperature:
        <span id="userTemperature">---</span>
	<span>C</span>
      </p>

      <p>
        Current Temperature of Drink:
        <span id="currentTemperature">---</span>
	<span>C</span>
      </p>      
      <button  onclick="sendFlag()">Press Here to Turn Off Buzzer</button>
    </section>
  </main>

  <script>
    async function fetchData() {
      try {
        const response = await fetch('/data', { cache: 'no-cache' });
        if (!response.ok) {
          console.log('HTTP error', response.status);
          return;
        }
        const data = await response.json();

        document.getElementById('userTemperature').textContent = data.temperatureValue;
        document.getElementById('currentTemperature').textContent = data.sensorTemperature;

        // myFlag is boolean -> show as true/false text
        document.getElementById('flagVal').textContent = data.myFlag ? 'true' : 'false';
      } catch (e) {
        console.log('Error fetching data:', e);
      }
    }

    async function sendFlag() {
      try {
        // POST request to inform Arduino that button was pressed
        const response = await fetch('/flag', {
          method: 'POST',
          headers: { 'Content-Type': 'text/plain' },
          body: 'pressed'
        });
        if (!response.ok) {
          console.log('HTTP error on flag', response.status);
        }
        // Optionally refresh displayed flag status
        fetchData();
      } catch (e) {
        console.log('Error sending flag:', e);
      }
    }

    // Initial load and periodic updates for sensor data + flag
    fetchData();
    setInterval(fetchData, 1000);
  </script>
</body>
</html>
)rawliteral";
