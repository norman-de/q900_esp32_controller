#ifndef HTML_H
#define HTML_H

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Radio Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: black;
            color: #33FF33;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 20;
        }
        .radio-container {
            background-color: #333;
            border: 3px solid #555;
            border-radius: 10px;
            padding: 20px;
            width: 300px;
            box-shadow: 0 0 20px rgba(0, 255, 0, 0.5);
        }
        h1 {
            text-align: center;
            font-size: 1.5em;
            color: #FFAA33;
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-top: 15px;
            font-size: 1em;
        }
        input[type="number"], select {
            width: 100%;
            padding: 5px;
            margin-top: 5px;
            font-size: 1em;
            color: #33FF33;
            background-color: #111;
            border: 1px solid #555;
            border-radius: 5px;
            text-align: right;
        }
        button {
            width: 45%;
            padding: 10px;
            margin: 5px 5px;
            font-size: 1em;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
        .on-button {
            background-color: #008000;
        }
        .off-button {
            background-color: #FF0000;
        }
        .display-text {
            font-family: 'Courier New', monospace;
            color: #33FF33;
            font-size: 1.2em;
            margin-top: 20px;
            text-align: center;
        }
        .favorites {
            margin-top: 20px;
            text-align: center;
        }
        .favorites button {
            width: 100%;
            margin-top: 5px;
            background-color: #008000;
        }
    </style>
</head>
<body>
    <div class="radio-container">
        <h1>Radio Control</h1>
        <div class="favorites">
            <button onclick="showBand()" style="width: auto; display: inline-block; color: #FFAA33; background-color: #333">&lt;Band&gt;</button>
            <button onclick="showBroadcast()" style="width: auto; display: inline-block; color: #FFAA33; background-color: #333">&ltBC&gt;</button>
            <button onclick="showFavorites()" style="width: auto; display: inline-block; color: #FFAA33; background-color: #333">&lt;Favorites&gt;</button>
            <div id="band-menu" style="display: none;">
                <button onclick="setFavorite(1.8, 1.8, 1, 39)">160m</button>
                <button onclick="setFavorite(3.5, 3.5, 1, 39)">80m</button>
                <button onclick="setFavorite(7.1, 7.1, 1, 39)">40m</button>
                <button onclick="setFavorite(14.2, 14.2, 1, 39)">20m</button>
                <button onclick="setFavorite(18.1, 18.1, 0, 39)">17m</button>
                <button onclick="setFavorite(21.2, 21.2, 0, 39)">15m</button>
                <button onclick="setFavorite(24.9, 24.9, 0, 39)">12m</button>
                <button onclick="setFavorite(28.4, 28.4, 0, 39)">10m</button>
                <button onclick="setFavorite(50.1, 50.1, 0, 39)">6m</button>
                <button onclick="setFavorite(144.3, 144.3, 6, 3)">2m</button>
                <button onclick="setFavorite(432.8, 432.8, 6, 3)">70cm</button>
                <button onclick="setFavorite(1297.494, 1297.494, 6, 3)">23cm</button>
            </div>
            <div id="broadcast-menu" style="display: none;">
                <button onclick="setFavorite(7.215, 7.215, 4, 83)">CHINA RADIO INTERNATIONAL</button>
            </div>
            <div id="favorites-menu" style="display: none;">
            <button onclick="setFavorite(27.285, 27.285, 6, 3)">CB 28</button>
                <button onclick="setFavorite(27.285, 27.285, 6, 3)">CB 28</button>
                <button onclick="setFavorite(27.335, 27.335, 6, 3)">CB 33</button>
                <!-- More favorites here -->
            </div>
        </div>
        <label for="vfoA">vfoA (MHz):</label>
        <input type="number" id="vfoA" step="0.0005" placeholder="Frequenz für VfoA">
        <label for="vfoB">vfoB (MHz):</label>
        <input type="number" id="vfoB" step="0.0005" placeholder="Frequenz für VfoB">
        <label for="mode">mode:</label>
        <select id="mode" onchange="updateFilterOptions()">
            <option value="0">USB</option>
            <option value="1">LSB</option>
            <option value="2">CWR</option>
            <option value="3">CWL</option>
            <option value="4">AM</option>
            <option value="5">WFM</option>
            <option value="6">NFM</option>
            <option value="7">DIGI</option>
            <option value="8">PKT</option>
        </select>
        <label for="abSwitch">a/b frequency:</label>
        <select id="abSwitch" onchange="setABFrequency()">
            <option value="0">A frequency</option>
            <option value="1">B frequency</option>
            <option value="2">A = B frequency</option>
        </select>
        <label for="filter">filter:</label>
        <select id="filter" onchange="setFilter()"></select>
        <div style="text-align: center; margin-top: 20px;">
            <button class="on-button" onclick="togglePower('on')">Switch On</button>
            <button class="off-button" onclick="togglePower('off')">Switch Off</button>
        </div>
        <div class="display-text">Set frequency and mode</div>
    </div>
    <script>
        const filters = {
            "FM": [
                { value: 1, text: "<7.2k" },
                { value: 2, text: "<10.0k" },
                { value: 3, text: "<12.0k" }
            ],
            "CW": [
                { value: 4, text: "<250_550" },
                { value: 5, text: "<250_575" },
                { value: 6, text: "<300_600" },
                { value: 7, text: "<325_625" },
                { value: 8, text: "<350_650" },
                { value: 9, text: "<375_675" },
                { value: 10, text: "<400_700" },
                { value: 11, text: "<425_725" },
                { value: 12, text: "<450_750" },
                { value: 13, text: "<475_775" },
                { value: 14, text: "<275_775" },
                { value: 15, text: "<325_825" },
                { value: 16, text: "<375_875" },
                { value: 17, text: "<425_925" },
                { value: 18, text: "<475_975" },
                { value: 19, text: "<0_1.4k" },
                { value: 20, text: "<370_1.7k" },
                { value: 21, text: "<0_1.6k" },
                { value: 23, text: "<500_2.3k" },
                { value: 24, text: "<600_2.4k" },
                { value: 25, text: "<700_2.5k" },
                { value: 26, text: "<800_2.6k" },
                { value: 28, text: "<0_1.8k" },
                { value: 29, text: "<0_2.1k" },
                { value: 30, text: "<500_2.6k" },
                { value: 31, text: "<600_2.9k" },
                { value: 33, text: "<800_3.1k" },
                { value: 34, text: "<900_3.2k" },
                { value: 35, text: "<0_2.3k" },
                { value: 36, text: "<0_2.5k" }
            ],
            "SSB": [
             { value: 4, text: "<250_550" },
                { value: 5, text: "<250_575" },
                { value: 6, text: "<300_600" },
                { value: 7, text: "<325_625" },
                { value: 8, text: "<350_650" },
                { value: 9, text: "<375_675" },
                { value: 10, text: "<400_700" },
                { value: 11, text: "<425_725" },
                { value: 12, text: "<450_750" },
                { value: 13, text: "<475_775" },
                { value: 14, text: "<275_775" },
                { value: 15, text: "<325_825" },
                { value: 16, text: "<375_875" },
                { value: 17, text: "<425_925" },
                { value: 18, text: "<475_975" },
                { value: 19, text: "<0_1.4k" },
                { value: 20, text: "<370_1.7k" },
                { value: 21, text: "<0_1.6k" },
                { value: 23, text: "<500_2.3k" },
                { value: 24, text: "<600_2.4k" },
                { value: 25, text: "<700_2.5k" },
                { value: 26, text: "<800_2.6k" },
                { value: 28, text: "<0_1.8k" },
                { value: 29, text: "<0_2.1k" },
                { value: 30, text: "<500_2.6k" },
                { value: 31, text: "<600_2.9k" },
                { value: 33, text: "<800_3.1k" },
                { value: 34, text: "<900_3.2k" },
                { value: 35, text: "<0_2.3k" },
                { value: 36, text: "<0_2.5k" },
                { value: 37, text: "<650_3.2k" },
                { value: 39, text: "<700_3.4k" },
                { value: 40, text: "<0_2.9k" },
                { value: 41, text: "<800_3.7k" },
                { value: 42, text: "<0_3.2k" },
                { value: 43, text: "<900_4.1k" },
                { value: 44, text: "<0_3.4k" },
                { value: 45, text: "<900_4.3k" },
                { value: 46, text: "<0_3.6k" },
                { value: 47, text: "<1.0k_4.6k" },
                { value: 48, text: "<0_3.8k" },
                { value: 49, text: "<1.1k_4.9k" },
                { value: 50, text: "<0_4.0k" },
                { value: 51, text: "<0_4.2k" },
                { value: 52, text: "<0_4.4k" },
                { value: 53, text: "<0_4.6k" },
                { value: 54, text: "<0_4.8k" },
                { value: 55, text: "<0_5.0k" },
                { value: 56, text: "<0_5.5k" },
                { value: 57, text: "<0_6.0k" },
                { value: 58, text: "<0_6.5k" },
                { value: 59, text: "<0_7.0k" },
                { value: 60, text: "<0_7.5k" },
                { value: 61, text: "<0_8.0k" },
                { value: 62, text: "<0_8.5k" },
                { value: 63, text: "<0_9.0k" },
                { value: 64, text: "<0_9.5k" },
                { value: 65, text: "<0_10.0k" }
            ],
            "AM": [
                { value: 66, text: "<1.4k" },
                { value: 67, text: "<1.6k" },
                { value: 68, text: "<1.8k" },
                { value: 69, text: "<2.0k" },
                { value: 70, text: "<2.3k" },
                { value: 71, text: "<2.5k" },
                { value: 72, text: "<2.7k" },
                { value: 73, text: "<2.8k" },
                { value: 74, text: "<3.2k" },
                { value: 75, text: "<3.4k" },
                { value: 76, text: "<3.6k" },
                { value: 77, text: "<3.8k" },
                { value: 78, text: "<4.0k" },
                { value: 79, text: "<4.2k" },
                { value: 80, text: "<4.4k" },
                { value: 81, text: "<4.6k" },
                { value: 82, text: "<4.8k" },
                { value: 83, text: "<5.0k" },
                { value: 84, text: "<6.0k" },
                { value: 85, text: "<7.5k" },
                { value: 86, text: "<10.0k" }
            ]
        };

        async function fetchData(url) {
            try {
                if(document.querySelector(".display-text").style.color !== "#33FF33") {
                    document.querySelector(".display-text").style.color = "#33FF33";
                    document.querySelector(".display-text").textContent = "";
                }
                const response = await fetch(url);
                if (response.status === 500) {
                    document.querySelector(".display-text").style.color = "red";
                    document.querySelector(".display-text").textContent = "Device Offline";
                    return;
                }
                return await response.json();
            } catch (error) {
                console.error('Fehler:', error);
                document.querySelector(".display-text").style.color = "red";
                document.querySelector(".display-text").textContent = "Fehler beim Laden";
            }
        }

        async function setFrequency() {
            const vfoA = document.getElementById('vfoA').value * 1e6;
            const vfoB = document.getElementById('vfoB').value * 1e6;
            const url = `/setFrequency?vfoa=${vfoA}&vfob=${vfoB}`;
            const data = await fetchData(url);
            console.log('Frequenz gesetzt:', data);
        }

        async function setMode() {
            const mode = document.getElementById('mode').value;
            const url = `/mode?modeA=${mode}&modeB=${mode}`;
            const data = await fetchData(url);
            console.log('Modus gesetzt:', data);
        }

        async function togglePower(action) {
            const url = `/${action}`;
            const data = await fetchData(url);
            console.log('Status:', data);
        }

        async function setABFrequency() {
            const ab = document.getElementById('abSwitch').value;
            const url = `/abFrequency?ab=${ab}`;
            const data = await fetchData(url);
            console.log('A/B Frequenz gesetzt:', data);
        }

        function updateFilterOptions() {
            const mode = document.getElementById('mode').value;
            const filterSelect = document.getElementById('filter');
            filterSelect.innerHTML = "";

            let modeFilters;
            if (mode === "4") {
                modeFilters = filters["AM"];
            } else if (["0", "1"].includes(mode)) {
                modeFilters = filters["SSB"];
            } else if (["2", "3"].includes(mode)) {
                modeFilters = filters["CW"];
            } else {
                modeFilters = filters["FM"];
            }

            modeFilters.forEach(filter => {
                const option = document.createElement("option");
                option.value = filter.value;
                option.textContent = filter.text;
                filterSelect.appendChild(option);
            });
        }

        async function setFilter() {
            const filter = document.getElementById('filter').value;
            const url = `/filter?filter=${filter}`;
            const data = await fetchData(url);
            console.log('Filter gesetzt:', data);
        }

        async function loadStatus() {
            const data = await fetchData("/getstatus");
            if (!data) return;
            if (data.VFOA_Frequency !== undefined) {
                document.getElementById("vfoA").value = (data.VFOA_Frequency / 1e6).toFixed(6);
            }
            if (data.VFOB_Frequency !== undefined) {
                document.getElementById("vfoB").value = (data.VFOB_Frequency / 1e6).toFixed(6);
            }
            if (data.VFOA_Mode !== undefined) {
                document.getElementById("mode").value = data.VFOA_Mode;
                updateFilterOptions();
            }
            if (data.Filter_Bandwidth !== undefined) {
                document.getElementById("filter").value = data.Filter_Bandwidth;
            }
            if (data.A_B !== undefined) {
                document.getElementById("abSwitch").value = data.A_B;
            }
            if (data.Voltage !== undefined) {
                document.querySelector(".display-text").textContent = `Spannung: ${data.Voltage} V`;
            }
        }

        function showFavorites() {
            const menu = document.getElementById('favorites-menu');
            menu.style.display = menu.style.display === 'none' ? 'block' : 'none';
            const band = document.getElementById('band-menu');
            band.style.display = 'none';
            const broadcast = document.getElementById('broadcast-menu');
            broadcast.style.display = 'none';
        }

        function showBand() {
            const band = document.getElementById('band-menu');
            band.style.display = band.style.display === 'none' ? 'block' : 'none';
            const menu = document.getElementById('favorites-menu');
            menu.style.display = 'none';
            const broadcast = document.getElementById('broadcast-menu');
            broadcast.style.display = 'none';
        }

        function showBroadcast() {
            const broadcast = document.getElementById('broadcast-menu');
            broadcast.style.display = broadcast.style.display === 'none' ? 'block' : 'none';
            const menu = document.getElementById('favorites-menu');
            menu.style.display = 'none';
            const band = document.getElementById('band-menu');
            band.style.display = 'none';
        }

        async function setFavorite(frequencyA, frequencyB, mode, filter) {
            document.getElementById('vfoA').value = frequencyA;
            document.getElementById('vfoB').value = frequencyB;
            await setFrequency();
            document.getElementById('mode').value = mode;
            await setMode();
            await updateFilterOptions()
            document.getElementById('filter').value = filter;
            await setFilter();
        }

        document.getElementById('vfoA').addEventListener('change', setFrequency);
        document.getElementById('vfoB').addEventListener('change', setFrequency);
        document.getElementById('mode').addEventListener('change', setMode);

        window.onload = loadStatus;
    </script>
</body>
</html>
)rawliteral";

#endif // HTML_H