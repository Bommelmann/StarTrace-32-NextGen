
async function populateTableDetectedECUs() {
    const tableBody = document.querySelector("#detected-ecus tbody");
    let AnythingDetected = false;
    // Objekt iterieren und Zeilen hinzufügen
    for (const device of globalConfigJSON.ECUs) {
        if (device.ECUDetected == true) {
            AnythingDetected = true;
            const row = document.createElement("tr"); // Neue Zeile erstellen
            const cellKey = document.createElement("td"); // Erste Zelle: Key
            cellKey.innerHTML = `${device.shortLabel} <span class="checkmark">&#x2714;</span>`;
            row.appendChild(cellKey);
            const cellValue = document.createElement("td"); // Zweite Zelle: Value
            cellValue.textContent = '0x' + device.DetectedDiagVersion;
            row.appendChild(cellValue);
            tableBody.appendChild(row); // Zeile zur Tabelle hinzufügen
        }
    }
    if (AnythingDetected == false) {
        const row = document.createElement("tr"); // Neue Zeile erstellen
        const cellKey = document.createElement("td"); // Erste Zelle: Key
        cellKey.textContent = "No ECU detected";
        row.appendChild(cellKey);
        tableBody.appendChild(row); // Zeile zur Tabelle hinzufügen
    }
}

async function populateTableCANConfig(){

    const tableBody = document.querySelector("#can-configuration tbody");

        let row = document.createElement("tr");
        let cellKey = document.createElement("td");
            let configuration=globalConfigJSON.CAN_Bus.rx_pin;
            cellKey.textContent = "Receive GPIO";
            row.appendChild(cellKey);
            let cellValue = document.createElement("td");
            cellValue.textContent = configuration;
            row.appendChild(cellValue);
            tableBody.appendChild(row);
        

        row = document.createElement("tr");
        cellKey = document.createElement("td");
            configuration=globalConfigJSON.CAN_Bus.tx_pin;
            cellKey.textContent = "Transmit GPIO";
            row.appendChild(cellKey);
            cellValue = document.createElement("td");
            cellValue.textContent = configuration;
            row.appendChild(cellValue);
            tableBody.appendChild(row);
        row = document.createElement("tr");
        cellKey = document.createElement("td");
            configuration=globalConfigJSON.CAN_Bus.baudrate;
            cellKey.textContent = "Data Transfer Rate";
            row.appendChild(cellKey);
            cellValue = document.createElement("td");
            cellValue.textContent = configuration +" Bit/s";
            row.appendChild(cellValue);
            tableBody.appendChild(row);
        
        row = document.createElement("tr");
        cellKey = document.createElement("td");
            configuration=globalConfigJSON.CAN_Bus.testerAddress;
            cellKey.textContent = "Tester Address";
            row.appendChild(cellKey);
            cellValue = document.createElement("td");
            cellValue.textContent = configuration;
            row.appendChild(cellValue);
            tableBody.appendChild(row);
        row = document.createElement("tr");
        cellKey = document.createElement("td");
            configuration=globalConfigJSON.CAN_Bus.ID_length_bit;
            cellKey.textContent = "ID Length";
            row.appendChild(cellKey);
            cellValue = document.createElement("td");
            cellValue.textContent = configuration +" Bit";
            row.appendChild(cellValue);
            tableBody.appendChild(row);  


}

async function populateTableWifi(){
        const tableBody = document.querySelector("#wifi-configuration tbody");
        let row = document.createElement("tr");
        let cellKey = document.createElement("td");
            let configuration=globalConfigJSON.WIFI.SSID;
            if(configuration==""){
                configuration="No SSID configured";
            }
            cellKey.textContent = configuration;
            row.appendChild(cellKey);
            let cellValue = document.createElement("td");
            configuration=globalConfigJSON.WIFI.password;
            if(configuration==""){
                configuration="No Password configured";
            }
            cellValue.textContent = configuration;
            row.appendChild(cellValue);
            tableBody.appendChild(row);

}

