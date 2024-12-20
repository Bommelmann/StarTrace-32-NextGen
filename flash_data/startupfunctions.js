async function getInitialAssets(){

    const link = "/config.json";

    // Definiere das Timeout (in Millisekunden)
    const timeout = 5000; // 5 Sekunden

    // Funktion, die ein Timeout zurückgibt
    function timeoutPromise() {
        return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
    }
        //Zuerst prüfen, ob sich ein Objekt namens "globalConfigJSON" im Cache befindet
        try{
            if(false){
            //const cachedConfig = localStorage.getItem('globalConfigJSON');
            //if ((cachedConfig !== undefined) && (cachedConfig !== null) && (cachedConfig !== "") && (cachedConfig != "{}")) {
            //    globalConfigJSON = JSON.parse(cachedConfig);
            } else {
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
            }   
            }catch(error){
                //Fehlerbehandlung   
                console.error('Fehler bei der Anfrage:', error);
                showErrorModal(error.message);

            }

}
//Object to verify which ECU is available and if, with which diagnostic version



async function ScanECUs() {
    console.log("ScanECUs");

    // Iteriere durch ECUs
    for (const device of globalConfigJSON.ECUs) {

        if (device.AutomaticConnect==true){
            console.log(`Device: ${device.shortLabel}`);
            
            // TA: Tester Address
            let TA = globalConfigJSON.CAN_Bus.testerAddress.toString(16).toUpperCase();
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
                const datajson = await DiagnosticRequest(request);  // Warten auf die Antwort von DiagnosticRequest
                console.log('Diag Response:', datajson);  // Antwort verwenden
                //Antwort ist schon json
                let resp = await checkforResp(datajson);  // Warten auf die Antwort von checkforResp
                
                if (resp=="NO_RESPONSE"){
                    device.ECUDetected=false;
                    console.log('DiagVers: NO_RESPONSE');
                }else if(resp=="NEGATIVE_RESPONSE"){
                    device.ECUDetected=true;
                    device.DetectedDiagVersion="";
                    console.log('DiagVers:', device.DetectedDiagVersion);
                }else if(resp=="POSITIVE_RESPONSE"){
                let diagvers=await checkforDiagVers(datajson, device);
                device.ECUDetected=true;
                device.DetectedDiagVersion=diagvers;
                console.log('DiagVers:', device.DetectedDiagVersion);
                }
            } catch (error) {
                console.error('Fehler beim Verarbeiten:', error);
                showErrorModal(error.message);
            }
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

async function getDiagDescriptions(){
    // Definiere das Timeout (in Millisekunden)
    const timeout = 3000; // 5 Sekunden

    // Funktion, die ein Timeout zurückgibt
    function timeoutPromise() {
        return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
    }
    
    try{
        //Wenn Cache vorhanden, gibt es keinen Grund die DiagDatenbanken neu zu laden
        if(false){
            if ((cachedConfig !== undefined) && (cachedConfig !== null) && (cachedConfig !== "") && (cachedConfig != "{}")) {
                globalConfigJSON = JSON.parse(cachedConfig); }
        } else {
            for (const device of globalConfigJSON.ECUs) {
                if(device.ECUDetected==true){
                    let filename= await getDiagDescriptionName(device);
                    const link="/sdcard?/DiagDescriptions/"+filename;                

                        const response = await Promise.race([
                            fetch(link, {
                                method: 'GET',
                                headers: {
                                    'Content-Type': 'application/gzip'
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
                        //Speichern der Diagnosedatenbank in der globalen Variable
                        device.DiagnosticDescription=data;

                }
            }
        }   
    }catch(error){
        //Fehlerbehandlung   
        console.error('Fehler bei der Anfrage:', error);
        showErrorModal(error.message);

    }
}


    async function getDiagDescriptionName(device){
        DiagnosticDescriptionNames=device.DiagnosticDescriptionNames;
        let Detecteddiagversion=device.DetectedDiagVersion;
        let FoundExactMatch=false;
        let closest_hex_value=-1;
        let temp_filename;
        //Durch DiagnosticDescriptionnames iterieren
        for (const description of device.DiagnosticDescriptionNames){
            Name=description.Name;
            //Wenn der Name die Diagnoseversion enthält, dann ist es die richtige Beschreibung
            if (Name.includes(Detecteddiagversion)){
                FoundExactMatch=true;
                description.Status="Loaded";
                console.log("DiagDescriptionName:", Name);
                return Name;
            }
            //Nächstmögliche DiagDescription laden
            else{         
                let Detecteddiagversionhex=parseInt(Detecteddiagversion, 16);
                let Readdiagversionhex;
                const hexIndex = Name.indexOf("0x");
                    if (hexIndex !== -1 && hexIndex + 6 <= Name.length) {
                        // Extrahiere die 4 Zeichen nach "0x"
                        Readdiagversionhex=parseInt(Name.substring(hexIndex + 2, hexIndex + 6),16);
                    }
                    else{
                        throw new Error(`DiagDescriptions with wrong Name Format!`);
                    }
                if (Readdiagversionhex>=0 && Readdiagversionhex<Detecteddiagversionhex&&(closest_hex_value ==-1 || Readdiagversionhex > closest_hex_value)){
                    closest_hex_value=Readdiagversionhex;
                    temp_filename=Name;
                }
            }

        }
        if(FoundExactMatch==false&&temp_filename!=undefined){ 
        console.log("DiagDescriptionName:", temp_filename);
        return temp_filename;        

        }
    }


async function TestGetDiagDescriptions(){
 // Definiere das Timeout (in Millisekunden)
 const timeout = 3000; // 5 Sekunden

 // Funktion, die ein Timeout zurückgibt
 function timeoutPromise() {
     return new Promise((_, reject) => setTimeout(() => reject(new Error('Timeout beim Abrufen der Daten')), timeout));
 }
 
            try{
                let filenames=[{"Name":"TCM01ST_0x0705.json.gz"},{"Name":"CPC04T_0x060E.json.gz"},{"Name": "MCM21T_0x0C13.json.gz"}, {"Name": "ACM301T_0x1F40.json.gz"} ];
      
                for (ArrayElement of filenames){
                    filename=ArrayElement.Name;
                    const link="/sdcard?/DiagDescriptions/"+filename;               

                    //Zuerst prüfen, ob sich ein Objekt namens "globalConfigJSON" im Cache befindet
                    const cachedConfig = localStorage.getItem('globalConfigJSON');

                        const response = await Promise.race([
                            fetch(link, {
                                method: 'GET',
                                headers: {
                                    'Content-Type': 'application/gzip'
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
                        console.log("data", data);
                        
                        //if(link.includes("CPC"))
                        // Speichern der erhaltenen Daten in der globalen Variable
                }       //globalConfigJSON.ECUs.DiagnosticDescription = data;
 
    }catch(error){
        //Fehlerbehandlung   
        console.error('Fehler bei der Anfrage:', error);
        showErrorModal(error.message);

    }



    }