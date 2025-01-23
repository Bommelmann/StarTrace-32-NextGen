async function getDiagDescriptions(){
    // Definiere das Timeout (in Millisekunden)
    const timeout = 3000; // 5 Sekunden
    let filename;

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
                    filename= await getDiagDescriptionName(device);
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

                        await addDiagnosticDescription(device, data);

                }
            }
        }   
    }catch(error){
        //Fehlerbehandlung   
        console.error('Fehler bei der Anfrage:', error);
        showErrorModal(error.message + " Diagnostic Description: " + filename);

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

// Funktion zum Hinzufügen einer neuen Diagnosedatenbank
function addDiagnosticDescription(device, data) {
    // Überprüfen, ob das Array bereits existiert, andernfalls erstellen
    if (!globalDiagDescriptions.diagnostics) {
        globalDiagDescriptions.diagnostics = [];
    }

    // Neues Objekt erstellen
    const newDescription = {
        shortLabel: device.shortLabel,
        DiagDescriptions: data
    };

    // Objekt zum Array hinzufügen
    globalDiagDescriptions.diagnostics.push(newDescription);
}



