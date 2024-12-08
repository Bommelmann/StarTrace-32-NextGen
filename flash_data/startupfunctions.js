async function getInitialAssets(){

    const link = "/config.json";

    // Definiere das Timeout (in Millisekunden)
    const timeout = 5000; // 5 Sekunden

    // Funktion, die ein Timeout zurückgibt
    function timeoutPromise() {
        return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
    }
        try{
            // Warte auf die erste der beiden Promises (fetch oder Timeout)
            const response = await Promise.race([
                fetch(link, {
                    method: 'GET',
                    headers: {
                        'Content-Type': 'application/json'
                    }
                }),
                timeoutPromise() // Timeout abwarten
            ]);

            // Prüfen, ob die Antwort erfolgreich war
            if (!response.ok) {
                throw new Error(`HTTP-Error! Status: ${response.status}`);
            }

            // Warten auf das Parsen der JSON-Antwort
            const data = await response.json(); 

            // Speichern der erhaltenen Daten in der globalen Variable
            globalConfigJSON = data;

            }catch(error){
                //Fehlerbehandlung   
                console.error('Fehler bei der Anfrage:', error);
                showErrorModal(error.message);

            }

}

async function ScanECUs() {
    console.log("ScanECUs");

    // Iteriere durch UDS_devices
    for (const device of globalConfigJSON.UDS_devices) {
        console.log(`Device: ${device.shortLabel}`);
        
        // TA: Tester Address
        let TA = globalConfigJSON.can.testerAddress.toString(16).toUpperCase();
        TA = TA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2
        
        // SA: Service Address
        let SA = device.SA.toString(16).toUpperCase();
        SA = SA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2
        
        // DiagVersionService und ComService
        let DiagVersionService = device.DiagVersionService;
        let ComService = DiagVersionService.ComService;
        
        // Erstelle request
        let request = "18DA" + SA + TA + ComService;
        console.log ("request:", request);

        try {
            const data = await DiagnosticRequest(request);  // Warten auf die Antwort von DiagnosticRequest
            console.log('Diag Response:', data);  // Antwort verwenden
        } catch (error) {
            console.error('Fehler beim Verarbeiten:', error);
            showErrorModal(error.message);
        }
    }
}


    // Funktion zum Anzeigen des Modals
    function showErrorModal(error) {
    const errorModal = document.getElementById('error-modal');
    const errorText = document.getElementById('error-text');
    errorText.innerText=error;
    errorModal.style.display = 'flex'; // Modal wird sichtbar
}

// Hilfsfunktion für Verzögerung
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

