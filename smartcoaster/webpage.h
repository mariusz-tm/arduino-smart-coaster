// webpage.h
#pragma once

const char MAIN_page[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Coaster Project</title>
<style>
  main {
    display: flex;
    flex-direction: column;
    align-items: center; 
  }

  body {
    background-color: #1C1919;
    color: white;
    font-family: Verdana, Geneva, Tahoma, sans-serif;
    margin: 0;
    padding: 0;
  }

  section {
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 15px;
    margin-top: 50px;
    text-align: center;
  }

  h2 {
    background-color: #FD5E53;
    padding: 10px 15px;
    border-radius: 5px;
    display: inline-block;
    font-size: 2em;
  }

  h3 {
    font-size: 0.9em;
    font-weight: normal;
    color: rgb(165, 159, 159);
  }

  a {
    font-size: 1em;
    font-family: Verdana, Geneva, Tahoma, sans-serif;
    color: white;
    text-decoration-color: #FD5E53;
    text-decoration-thickness: 1px;
    margin: 0;
  }

  button {
    padding: 8px 18px;
    border: 5px solid #FD5E53;
    border-radius: 25px;
    background-color: #f78c84;
    font-size: 1em;
    color: white;
  }

  button:active {
    border: 5px solid #481916;
    background-color: #8a2c26;
    color: gray;
  }

  .mainBox {
    width: 31.25%;
    border: 3px solid #FD5E53;
    border-radius: 10px;
    background-color: #161111;
    box-sizing: border-box;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    margin-top: 10px;
    min-height: 40vh;
    text-align: center;
  }

  /* 6.7" Screen Layout */
  @media screen and (max-width: 500px) {
    section {
      flex-direction: column;
      margin-top: 25px;
      gap: 10px;
    }

    h2 {
      font-size: 1.6em;
      padding: 8px 12px;
    }

    h3 {
      font-size: 0.8em;
      margin: 0 10px;
      line-height: 1.4em;
    }

    .mainBox {
      width: 90%;
      border-width: 2px;
      min-height: 35vh;
      padding: 10px 5px;
    }

    button {
      font-size: 0.9em;
      padding: 6px 12px;
      border-width: 3px;
    }

    a {
      font-size: 0.9em;
    }
  }
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
      </p>

      <p>
        Current Temperature of Drink:
        <span id="currentTemperature">---</span>
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
