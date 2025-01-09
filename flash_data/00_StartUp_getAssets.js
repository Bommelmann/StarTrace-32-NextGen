//Common function to get an asset from the server
async function getAsset(link, Content_Type){

    // Definiere das Timeout (in Millisekunden)
    const timeout = 3000; // 5 Sekunden

    // Funktion, die ein Timeout zurückgibt
    function timeoutPromise() {
        return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
    }
        //Zuerst prüfen, ob sich ein Objekt namens "globalConfigJSON" im Cache befindet
        try{

                const response = await Promise.race([
                    fetch(link, {
                        method: 'GET',
                        headers: {
                            'Content-Type': Content_Type
                        }
                    }),
                    timeoutPromise() // Timeout abwarten
                ]);
            

            // Prüfen, ob die Antwort erfolgreich war
            if (!response.ok) {
                throw new Error(`HTTP-Error! Status: ${response.status}` + " Link: " + link);
            }

            return response;
            
            }catch(error){
                //Fehlerbehandlung   
                console.error('Fehler bei der Anfrage:', error);
                showErrorModal(error.message);

            }

}

async function getElements() {
    return {
        closeBtn: document.getElementById('close-btn'),
        errorModal: document.getElementById('error-modal'),
        disclaimer_modal: document.getElementById('disclaimer-modal'),
        disclaimer_continue: document.getElementById('disclaimer-continue'),
        scanning_modal: document.getElementById('ecu-scan-modal'),
        ecuScanTitle: document.getElementById('ecu-scan-title'),
        ecuScanIcon: document.getElementById('ecu-scan-icon')
    };
}

async function loadHomeContent() {
    const response = await getAsset('home.html', 'text/html');
    const homeContent = await response.text();
    document.getElementById('home').innerHTML = homeContent;
}