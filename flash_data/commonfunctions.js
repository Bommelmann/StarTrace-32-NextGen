async function DiagnosticRequest(request) {
    const requestData = {
        "request": request,
        "response": ""
    };

    const link = "/uds_request?";

    // Definiere das Timeout (in Millisekunden)
    const timeout = 3000; // 3 Sekunden

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
        showErrorModal(error.message);
        throw error;  // Fehler weitergeben, wenn nötig
    }
}



// Function to proceed to the Identification page
function proceedToIdentification() {
    // Get selected ECU values
    const selectedECUs = Array.from(document.querySelectorAll('input[name="ecu"]:checked'))
                                .map(input => input.value);
    
    if (selectedECUs.length === 0) {
        alert('Please select at least one ECU to continue.');
    } else {
        // Close ECU selection modal and navigate to "Identification" section
        document.getElementById('ecu-modal').style.display = 'none';
        showContent('identification');
    }
}

// Function to show selected content and hide others
function showContent(section) {
    const sections = document.querySelectorAll('.content-section');
    sections.forEach(sec => sec.classList.add('hidden'));
    document.getElementById(section).classList.remove('hidden');
}

// Function to fetch JSON from server
function fetchJson() {
    fetch('/get-date')
        .then(response => response.json())
        .then(data => {
            document.getElementById('json-output').textContent = JSON.stringify(data);
        })
        .catch(error => {
            document.getElementById('json-output').textContent = 'Error fetching JSON';
            console.error('Error:', error);
        });
}
