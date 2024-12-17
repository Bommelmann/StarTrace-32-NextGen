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

//Response Rückgabewerte für "checkforResp()"
const ResponseEnum = {
    POSITIVE_RESPONSE: 'POSITIVE_RESPONSE',
    NEGATIVE_RESPONSE: 'NEGATIVE_RESPONSE',
    NO_RESPONSE: 'NO_RESPONSE'
};

async function checkforResp(datajson){
    //Extract 5th Byte in Request
    let request=datajson.request;
    console.log("RequestString", request);
    let reqchars=request.substring(8,10);
    //Extract 5th Byte in Response
    let response=datajson.response;
    console.log("ResponseString", response);
    let respchars=response.substring(8,10);
    //Extract possible NRC in Response
    let nrcchars=response.substring(12,14);
    console.log("NRCString", nrcchars);
    //Then compare request and response
    //First convert into Hex:
    let reqhex=parseInt(reqchars,16)
    let resphex=parseInt(respchars,16)
    let nrchex=parseInt(nrcchars,16)
    //Positive Response
    if ((reqhex|0x40)==resphex){
        return ResponseEnum.POSITIVE_RESPONSE;
    }
    if (resphex==0x7F){
        if (nrchex==0xFF){
            return ResponseEnum.NO_RESPONSE;
        }else{
            return ResponseEnum.NEGATIVE_RESPONSE;

        }
        
    }


}

async function checkforDiagVers(datajson, device){

    //Get Byte and Bit Positions out of config.json
    let BytePos=device.DiagVersionService.Datatypes.BytePos;
    let BitPos=device.DiagVersionService.Datatypes.BitPos;
    let ByteLength=device.DiagVersionService.Datatypes.ByteLength;
    let BitLength=device.DiagVersionService.Datatypes.BitLength;
    let ComServiceLength=device.DiagVersionService.ComService.length;
    let IDLength=globalConfigJSON.CAN_Bus.ID_length_bit;
    //Extract Chars of DiagVersion
    if(IDLength==29){
        let DiagVersStartChar=8+ComServiceLength+BytePos/2;
        let DiagVersEndChar=DiagVersStartChar+ByteLength*2;
        let DiagVers=datajson.response.substring(DiagVersStartChar,DiagVersEndChar);
        console.log("DiagVers", DiagVers);
        return DiagVers;
    }
    
}