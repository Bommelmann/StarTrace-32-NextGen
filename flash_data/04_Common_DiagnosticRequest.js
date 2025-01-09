async function DiagnosticRequest(request) {
    const requestData = {
        "request": request,
        "response": ""
    };

    const link = "/uds_request?";

    // Definiere das Timeout (in Millisekunden)
    const timeout = 5000; // 5 Sekunden

    // Funktion, die ein Timeout zurückgibt
    function timeoutPromise() {
        return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
    }

    try {
        // Warte auf die erste der beiden Promises (fetch oder Timeout)
        const response = await Promise.race([
            fetch(link, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(requestData)
            }),
            timeoutPromise() // Timeout abwarten
        ]);

        // Prüfen, ob die Antwort erfolgreich war
        if (!response.ok) {
            throw new Error(`HTTP-Error! Status: ${response.status}`);
        }

        // Warten auf das Parsen der JSON-Antwort
        const data = await response.json();

        // Rückgabe der Antwortdaten
        //console.log('Antwort vom Server:', data);
        return data;

    } catch (error) {
        // Fehlerbehandlung
        console.error('Fehler bei der Anfrage:', error);
        showErrorModalLight(error.message + ". DiagService: " + request);

        const artificialresponseData = {
            "request": request,
            "response": "Unknown"
        };
        return artificialresponseData;
        //throw error;  // Fehler weitergeben, wenn nötig
    }
}