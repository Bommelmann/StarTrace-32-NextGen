async function HandleIdentifications() {
    //Variable, mit der der erste Tab nur 1 Mal am Anfang gezeigt wird.
    let isContentShown=false;
    // First Check if the Diagnostic Descriptions are available
    // If not, show Error Modal
    if (globalDiagDescriptions.diagnostics.length > 0) {
        for (const ECUDiagDescriptions of globalDiagDescriptions.diagnostics) {
            //Create a Tab Buttonfor each ECU
            await createTabButton('tabs-container-identification',ECUDiagDescriptions.shortLabel, 'identification');
            //Create a Parent Container for each ECU
            await createContent ('identification', ECUDiagDescriptions.shortLabel);   
            // Show the Content of the Identification Tab
                //Because this function always shows the first tab, we only want to call it once
                if (isContentShown==false){
                    await showContent('identification');
                    isContentShown=true;
                }
            //If the wait modal is visible, hide it
            const errorModalWait = document.getElementById('error-modal-wait-identification');
            errorModalWait.style.display = 'none'; // Modal schließen
            // Iterate through the Identifications
            for (const Identification of ECUDiagDescriptions.DiagDescriptions.identifications) {
                // Aufgabe 1////////////////////////////////////////////
                // Diagnoserequest bauen
                let Identifier = await getIdentifier(ECUDiagDescriptions.shortLabel);
                let ComService = Identification.ComService.replace(/\s/g, "");
                let response = await DiagnosticRequest(Identifier + ComService);
                // Aufgabe 1.1////////////////////////////////////////////
                // Identification Überschrift im Tab anzeigen
                await createHeading(ECUDiagDescriptions.shortLabel+'identification', Identification.ServiceName + ' ' + ECUDiagDescriptions.shortLabel);
                // Aufgabe 2////////////////////////////////////////////
                // Diagnosedaten Interpretieren
                // Iteriere durch die einzelnen Datentypen der Identification
                for (const DataType of Identification.Response.DataTypes) {

                //Testzwecke
                if (Identification.ServiceName=="ReadF442 ECC System Voltage"){
                    console.log(response.response);
                    }
                    //Daten interpretieren
                    let Data = await interpretData(response.response, DataType);
                    //Interpretierte Daten in Diagnosedatenbanken unter "Result" speichern
                    DataType.Result = Data;
                    //Aufgabe 3////////////////////////////////////////////
                    //Diagosedaten Eintragen
                    await createDataEntry(Identification.ServiceName + ' ' + ECUDiagDescriptions.shortLabel, DataType);
                        //Aufgabe 3.1////////////////////////////////////////////
                        //Dropdown Menü unterhalb der Tabbuttons erweitern
                        await createTabButtonDropDown('dropdown-content'+ECUDiagDescriptions.shortLabel+'identification',DataType.DataName);
                    //Diagnosedaten Anzeigen
                    // Check if the tab is currently active
                    const isActiveTab = document.querySelector(`.tablink.active`)?.textContent === ('Electronic Control Unit: '+ECUDiagDescriptions.shortLabel);
                    // Wenn der Tab aktiv ist, zeige den neuen Inhalt sofort an
                    if (isActiveTab) {
                        let tmptab = document.getElementById(ECUDiagDescriptions.shortLabel+'identification');
                        await showAllChildren(tmptab);
                    }

                }
            }
            
        }
    } else {
        showErrorModal("Not possible to perform diagnostics, due to missing Diagnostic Descriptions");
    }
}

async function getIdentifier(shortLabel) {
    let SA;
    let TA;
    for (let device of globalConfigJSON.ECUs) {
        if ((device.shortLabel == shortLabel) && device.ECUDetected == true) {
            SA = device.SA.toString(16).toUpperCase();
            SA = SA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2
        }
    }
    TA = globalConfigJSON.CAN_Bus.testerAddress.toString(16).toUpperCase();
    TA = TA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2

    // Für den Fall, dass die ID Länge 29 Bit beträgt
    if (globalConfigJSON.CAN_Bus.ID_length_bit == 29) {
        // Erstelle request
        return "18DA" + SA + TA;
    }
}
