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

    // Funktion zum Abrufen der Daten
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

        //////////////Fehlerbehandlung/////////////////////////
            //Tritt ein in folgenden Fällen:
                //Fetch schlägt fehl
                    //z.B. Interner Server Fehler wie Uri nicht vorhanden
                //Timeout wird erreicht
                //Fehler beim Parsen der JSON Antwort
            //So wird der Fehler behandelt:
                //ErrorModal Light am unteren Bildschirmrand wird angezeigt
                //Fehlermeldung wird in der Konsole ausgegeben
                //Ein künstliches responseData wird erstellt, welches den Fehler enthält (0xFE)
            //Hinweis:
                //Wiederholen der Anfrage ist hier nicht nötig, da es es im Server eh gravierendes Problem gibt
        } catch (error) {
            // Fehlerbehandlung
            console.error('Fehler bei der Anfrage:', error);
            //showErrorModalLight(error.message + ". DiagService: " + request);
            showErrorModalLight("Lost vehicle connection! Is the Voltage Supply on?");

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
    const responsePattern = /7F..21/;

    while (attempts < maxAttempts) {
        data = await fetchRequest();
        if ((data.response !== "FE")||!(responsePattern.test(data.response))) {
            break;
        }
        //////////////Fehlerbehandlung/////////////////////////
            //Tritt ein in folgenden Fällen:
                //Alle Fehler außer:
                    //Fetch schlägt fehl
                    //Timeout wird erreicht
                    //Fehler beim Parsen der JSON Antwort
                //Außerdem:
                    //Die Antwort enthält den Fehlercode "Busy Repeat Request" (0x21)
            //So wird der Fehler behandelt:
                //Fehlermeldung wird in der Konsole ausgegeben
                //Anfrage wird wiederholt (weitere 2 Mal)
            //Hinweis:
                //Wiederholen der Anfrage ist hier nötig, da es sich um einen temporären Fehler handelt

        attempts++;
        console.log(`Retrying request (${attempts}/${maxAttempts}) due to failure.`);
        if(attempts==maxAttempts){
            showErrorModalLight("Maximale Anzahl an Versuchen erreicht. DiagService: " + request);
            data.response = "FF";
        }
    }

    return data;
}