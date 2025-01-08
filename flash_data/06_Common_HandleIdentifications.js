async function HandleIdentifications() {
    // First Check if the Diagnostic Descriptions are available
    // If not, show Error Modal
    if (globalDiagDescriptions.diagnostics.length > 0) {
        for (const ECUDiagDescriptions of globalDiagDescriptions.diagnostics) {
            //Create a Tab Buttonfor each ECU
            await createTabButton(ECUDiagDescriptions.shortLabel);
            //Create a Parent Container for each ECU
            await createContent ('identification', ECUDiagDescriptions.shortLabel);   

            // Iterate through the Identifications
            for (const Identification of ECUDiagDescriptions.DiagDescriptions.identifications) {
                // Aufgabe 1////////////////////////////////////////////
                // Diagnoserequest bauen
                let Identifier = await getIdentifier(ECUDiagDescriptions.shortLabel);
                let ComService = Identification.ComService.replace(/\s/g, "");
                let response = await DiagnosticRequest(Identifier + ComService);
                    // Aufgabe 1.1////////////////////////////////////////////
                    // Identification Überschrift im Tab anzeigen
                    await createHeading(ECUDiagDescriptions.shortLabel, Identification.ServiceName);
                // Aufgabe 2////////////////////////////////////////////
                // Diagnosedaten Interpretieren
                // Iteriere durch die einzelnen Datentypen der Identification
                for (const DataType of Identification.Response.DataTypes) {
                    //Daten interpretieren
                    let Data = await interpretData(response.response, DataType);
                    //Interpretierte Daten in Diagnosedatenbanken unter "Result" speichern
                    DataType.Result = Data;
                    //Aufgabe 3////////////////////////////////////////////
                    //Diagosedaten Eintragen
                    await createDataEntry(Identification.ServiceName, DataType);
                    //Diagnosedaten Anzeigen
                    // Check if the tab is currently active
                    const isActiveTab = document.querySelector(`.tablink.active`)?.textContent === ECUDiagDescriptions.shortLabel;
                    // Wenn der Tab aktiv ist, zeige den neuen Inhalt sofort an
                    if (isActiveTab) {
                        let tmptab = document.getElementById(ECUDiagDescriptions.shortLabel);
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
