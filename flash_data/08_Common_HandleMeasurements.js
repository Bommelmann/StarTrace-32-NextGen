async function HandleMeasurements() {
    // First Check if the Diagnostic Descriptions are available
    // If not, show Error Modal
    if (globalDiagDescriptions.diagnostics.length > 0) {
        for (const ECUDiagDescriptions of globalDiagDescriptions.diagnostics) {
            //Create a Tab Buttonfor each ECU
            await createTabButton('tabs-container-measurement',ECUDiagDescriptions.shortLabel,'measurement');
            //Create a Parent Container for each ECU
            await createContent ('measurement', ECUDiagDescriptions.shortLabel);   
            // Show the Content of the Measurement Tab
            //await showContent('measurement');
            //If the wait modal is visible, hide it
            const errorModalWait = document.getElementById('error-modal-wait');
            errorModalWait.style.display = 'none'; // Modal schließen
            // Iterate through the Identifications
            for (const measurement of ECUDiagDescriptions.DiagDescriptions.measurements) {
                // Aufgabe 1////////////////////////////////////////////
                // Diagnoserequest bauen
                let Identifier = await getIdentifier(ECUDiagDescriptions.shortLabel);
                let ComService = measurement.ComService.replace(/\s/g, "");
                let response = await DiagnosticRequest(Identifier + ComService);
                // Aufgabe 1.1////////////////////////////////////////////
                // Identification Überschrift im Tab anzeigen
                await createHeading(ECUDiagDescriptions.shortLabel +'measurement', measurement.ServiceName + ' ' + ECUDiagDescriptions.shortLabel);
                // Aufgabe 2////////////////////////////////////////////
                // Diagnosedaten Interpretieren
                // Iteriere durch die einzelnen Datentypen der Identification
                for (const DataType of measurement.Response.DataTypes) {
                    //Daten interpretieren
                    let Data = await interpretData(response.response, DataType);
                    //Interpretierte Daten in Diagnosedatenbanken unter "Result" speichern
                    DataType.Result = Data;
                    //Aufgabe 3////////////////////////////////////////////
                    //Diagosedaten Eintragen
                    await createDataEntry(measurement.ServiceName + ' ' + ECUDiagDescriptions.shortLabel, DataType);
                        //Aufgabe 3.1////////////////////////////////////////////
                        //Dropdown Menü unterhalb der Tabbuttons erweitern
                        await createTabButtonDropDown('dropdown-content'+ECUDiagDescriptions.shortLabel+'measurement',DataType.DataName);

                    //Diagnosedaten Anzeigen
                    // Check if the tab is currently active
                    const isActiveTab = document.querySelector(`.tablink.active`)?.textContent === (ECUDiagDescriptions.shortLabel);
                    // Wenn der Tab aktiv ist, zeige den neuen Inhalt sofort an
                    if (isActiveTab) {
                        let tmptab = document.getElementById(ECUDiagDescriptions.shortLabel+'measurement');
                        await showAllChildren(tmptab);
                    }

                }
            }
            
        }
    } else {
        showErrorModal("Not possible to perform diagnostics, due to missing Diagnostic Descriptions");
    }
}