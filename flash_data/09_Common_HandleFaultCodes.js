async function HandleFaultCodes() {
    // First Check if the Diagnostic Descriptions are available
    // If not, show Error Modal
    if (globalDiagDescriptions.diagnostics.length > 0) {
        for (const ECUDiagDescriptions of globalDiagDescriptions.diagnostics) {
            //Create a Tab Buttonfor each ECU
            await createTabButton('tabs-container-faultcodes',ECUDiagDescriptions.shortLabel,'faultcodes');
            //Create a Parent Container for each ECU
            await createContent ('faultcodes', ECUDiagDescriptions.shortLabel);   
            // Show the Content of the Measurement Tab
            await showContent('faultcodes');
            //If the wait modal is visible, hide it
            const errorModalWait = document.getElementById('error-modal-wait-faultcodes');
            errorModalWait.style.display = 'none'; // Modal schließen
        
            // Aufgabe 1////////////////////////////////////////////
            // Diagnoserequest ReadDataByStatus bauen
            let Identifier = await getIdentifier(ECUDiagDescriptions.shortLabel);
            let ComService;
            let ResponseDataType;
            let DTCArray;
            for (faultmemoryservice of ECUDiagDescriptions.DiagDescriptions.faultmemory.services){
                if (faultmemoryservice.ServiceName=="ReadDTCByStatus"){
                    ComService =faultmemoryservice.ComService.replace(/\s/g, "");
                    ResponseDataType = faultmemoryservice.Response.DataTypes[0];
                    // Aufgabe 1.1////////////////////////////////////////////
                    //Diagnoserequest ausführen
                    let response = await DiagnosticRequest(Identifier + ComService);
                    // Aufgabe 1.2////////////////////////////////////////////
                    // Active DTCs in einen Array schreiben           
                    DTCArray = await interpretData(response.response,ResponseDataType);
                    // In Diagnosedatenbank schreiben
                    ResponseDataType.Result = DTCArray;
                }
            }
            // Aufgabe 2////////////////////////////////////////////
            //Diagnoserequest ReadDTCExtendedDataRecord
            //ComService holen
            for (faultmemoryservice of ECUDiagDescriptions.DiagDescriptions.faultmemory.services){
                if (faultmemoryservice.ServiceName=="ReadDTCExtendedDataRecord"){
                    ComService =faultmemoryservice.ComService.replace(/\s/g, "");

                //Aufgabe 2.1////////////////////////////////////////////
                //Durch DTCArray iterieren und für jeden DTC den erweiterten Fehlerspeicher auslesen
                for (let DTC of DTCArray){
                    //Aufgabe 2.1.1////////////////////////////////////////////
                    //Durch RequestParameter iterieren und jeweils Daten zum Comservice hinzufügen
                    for (const RequestDataType of faultmemoryservice.Request.DataTypes){
                        // Da der Datentyp Name immer unterschiedlich ist, müssen wir diesen erstmal ermitteln
                        const PresentationsKey = Object.keys(RequestDataType.Presentations);
                        //Erstmal BytePos etc. ermitteln, um damit weitere FUnktionen wie "insertDataIntoComService" aufrufen können
                        let BytePos=RequestDataType.Presentations[PresentationsKey[0]]["Byte Pos"];
                        let BitPos=RequestDataType.Presentations[PresentationsKey[0]]["Bit Pos"];
                        let BitLength=RequestDataType.Presentations[PresentationsKey[0]]["Bit Length"];
                        //Request Parameter DTC
                        if (RequestDataType.DataName=="DTC"){
                            ComService = insertDataIntoComService(ComService, DTC, BytePos, BitPos, BitLength);
                        }
                        //Request Parameter Extended Data Record Number
                        else if(RequestDataType.DataName=="Extended Data Record Number"){
                            //Extended DataRecord Number ermitteln
                            let ExtendedDataRecordNumber;
                            if(RequestDataType.Presentations[PresentationsKey[0]].Convselector=="NONE"){
                                ExtendedDataRecordNumber=RequestDataType.Presentations[PresentationsKey[0]].DATA_HEX_STRING;
                                }
                            ComService = insertDataIntoComService(ComService, ExtendedDataRecordNumber, BytePos, BitPos, BitLength);
                        }

                    }
                    //Aufgabe 2.2
                    //Diagnoserequest ausführen
                    let response = await DiagnosticRequest(Identifier + ComService);
                    ComService=faultmemoryservice.ComService.replace(/\s/g, "");

                    //Aufgabe 2.3////////////////////////////////////////////
                    //Diagnosedaten Interpretieren
                    //Aufgabe 2.3.1////////////////////////////////////////////
                    //DTC Datentyp interpretieren
                    //Zuerst muss der DTC ermittelt werden, da das für die Extended Data Records gebraucht wird
                    let ResponseDataType;
                    for (const IterResponseDataType of faultmemoryservice.Response.DataTypes){
                        if (IterResponseDataType.DataName=="DTC"){
                            ResponseDataType=IterResponseDataType;
                        }
                    }
                    let ExtendedDataDTC = await interpretDTCData(response.response,ResponseDataType,"FFFFFF",ECUDiagDescriptions.shortLabel);
                    //In Diagnosedatenbank schreiben
                    ResponseDataType.Result = ExtendedDataDTC;
                    //Aufgabe 2.3.2////////////////////////////////////////////
                    //Extended Data Record Diagnosedaten Interpretieren
                    for (const IterResponseDataType of faultmemoryservice.Response.DataTypes){
                        if (IterResponseDataType.DataName=="Extended Data Record"){
                            ResponseDataType=IterResponseDataType;
                        }
                    }
                    let ExtendedDataObject= await interpretDTCData(response.response,ResponseDataType,DTC,ECUDiagDescriptions.shortLabel);
                        await createHeading(ECUDiagDescriptions.shortLabel +'faultcodes', Object.keys(ExtendedDataObject)[0]);
                        await createTabButtonDropDown('dropdown-content'+ECUDiagDescriptions.shortLabel+'faultcodes',Object.keys(ExtendedDataObject)[0]);
                        let isActiveTab = document.querySelector(`.tablink.active`)?.textContent === ('Electronic Control Unit: '+ECUDiagDescriptions.shortLabel);
                        // Wenn der Tab aktiv ist, zeige den neuen Inhalt sofort an
                        if (isActiveTab) {
                            let tmptab = document.getElementById(ECUDiagDescriptions.shortLabel+'faultcodes');
                            await showAllChildren(tmptab);
                        }               
                        for (let ExtendedDataElementElement of ExtendedDataObject[Object.keys(ExtendedDataObject)[0]]["Environment Data"]){
                            await createDataEntryFaultCodes(Object.keys(ExtendedDataObject)[0],ExtendedDataElementElement);
                            isActiveTab = document.querySelector(`.tablink.active`)?.textContent === ('Electronic Control Unit: '+ECUDiagDescriptions.shortLabel);
                            // Wenn der Tab aktiv ist, zeige den neuen Inhalt sofort an
                            if (isActiveTab) {
                                let tmptab = document.getElementById(ECUDiagDescriptions.shortLabel+'faultcodes');
                                await showAllChildren(tmptab);
                            }
                        }
                    
                    //In Diagnosedatenbank schreiben
                    // Stellen Sie sicher, dass ResponseDataType.Result ein Array ist
                    if (!Array.isArray(ResponseDataType.Result)) {
                        ResponseDataType.Result = [];
                    }
                    // Fügen Sie ExtendedDataObject als Array-Element hinzu
                    ResponseDataType.Result.push(ExtendedDataObject);
                }
            }
            }          
            
            
        }
    } else {
        showErrorModal("Not possible to perform diagnostics, due to missing Diagnostic Descriptions");
    }
}

function insertDataIntoComService(ComService, DTC, BytePos, BitPos, BitLength) {
    // Convert ComService to a binary string
    let comServiceBinary = BigInt('0x' + ComService).toString(2).padStart(ComService.length * 4, '0');
    // Convert DTC to a binary string
    let dtcBinary = BigInt('0x' + DTC).toString(2).padStart(BitLength, '0');

    // Calculate the start and end positions for the DTC insertion
    let startBit = BytePos * 8 + BitPos;
    let endBit = startBit + BitLength;

    // Insert the DTC into the specified position
    comServiceBinary = comServiceBinary.slice(0, startBit) + dtcBinary + comServiceBinary.slice(endBit);

    // Convert the binary string back to a hexadecimal string
    let comServiceHex = BigInt('0b' + comServiceBinary).toString(16).padStart(ComService.length, '0').toUpperCase();

    return comServiceHex;
}