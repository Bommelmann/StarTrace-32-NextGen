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

    async function fetchRequest() {
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
            return data;

        } catch (error) {
            // Fehlerbehandlung
            console.error('Fehler bei der Anfrage:', error);
            showErrorModalLight(error.message + ". DiagService: " + request);

            const artificialresponseData = {
                "request": request,
                "response": "FE"
            };
            return artificialresponseData;
        }
    }

    let data;
    let attempts = 0;
    const maxAttempts = 3;

    while (attempts < maxAttempts) {
        data = await fetchRequest();
        if (data.response !== "FE") {
            break;
        }
        attempts++;
        console.log(`Retrying request (${attempts}/${maxAttempts}) due to failure.`);
        if(attempts==maxAttempts){
            showErrorModalLight("Maximale Anzahl an Versuchen erreicht. DiagService: " + request);
            data.response = "FF";
        }
    }

    // Überprüfen, ob die Antwort "7F [any two digits] 21" enthält
    const responsePattern = /7F..21/;
    if (responsePattern.test(data.response)) {
        console.log("Retrying request due to response containing '7F [any two digits] 21'");
        data = await fetchRequest();
    }

    return data;
}