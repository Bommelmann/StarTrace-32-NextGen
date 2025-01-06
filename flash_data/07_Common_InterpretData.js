
async function interpretData(response, DataType) {
    let responseWithoutIdentifier;
    // Entferne die ersten 4 Byte (den Identifier), wenn IDlength ==29
    if (globalConfigJSON.CAN_Bus.ID_length_bit == 29) {
    responseWithoutIdentifier = response.substring(8);
    }


    // Da der Datentyp Name immer unterschiedlich ist, müssen wir diesen erstmal ermitteln
    const PresentationsKey = Object.keys(DataType.Presentations);
    // Wir müssen die Response erstmal in eine dezimale Zahl wandeln
    let responseDec = parseInt(responseWithoutIdentifier, 16);
    // Nun können wir die Response interpretieren
    // Zuerst müssen wir den Wert extrahieren, an der richtigen Stelle von der Response
    let RawData = await extractValue(responseDec, parseInt(DataType.Presentations[PresentationsKey[0]]["Byte Pos"],10), parseInt(DataType.Presentations[PresentationsKey[0]]["Bit Pos"],10), parseInt(DataType.Presentations[PresentationsKey[0]]["Bit Length"],10));
    let ProcessedData;

    // Verschiedene Datentypen benötigen verschiedene Interpretationen
    // Texttable (ConversionScale)
    if ((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_SCALE")) {
        ProcessedData = await interpretConvScale (RawData, DataType,PresentationsKey);
    }
    // Blanke Dezimal-Zahl (ConversionRaw)
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_RAW")) {
        ProcessedData = RawData.toString();
    }
    // ASCII (ConversionASCII)
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_ASCII")) {
        ProcessedData= String.fromCharCode(RawData);
    }
    // Hex Code (Conversion_DUMP)
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_DUMP")) {
        ProcessedData= "0x" + RawData.toString(16).toUpperCase();
    }
    // Conversion Factor Offset ist Schwachsinnsdatentyp aus der Diagnosedatenbank, wird gleich behandelt wie Raw
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_FACTOR_OFFSET")) {
        ProcessedData = RawData.toString();
    }
    else{
        showErrorModal("Datentyp aus Diagnosedatenbank unbekannt/nicht implementiert: "+ (DataType.Presentations[PresentationsKey[0]].Convselector));
    }
    
    //Nach Interpretation der Daten, diese zurück an den Aufrufer geben
    return ProcessedData;
}

async function extractValue(decimalNumber, bytePosition, bitPosition, bitLength) {
    // Konvertiere die Dezimalzahl in eine Binärdarstellung (mindestens 8 Bits je Byte)
    const binaryString = decimalNumber.toString(2).padStart(32, '0'); // für 32-Bit-Integer
    const startBit = bytePosition * 8 + bitPosition; // Berechne den Startbit-Index
    const endBit = startBit + bitLength; // Endbit-Index

    // Extrahiere die relevanten Bits
    const extractedBits = binaryString.slice(startBit-1, endBit-1);

    // Konvertiere die extrahierten Bits zurück in Dezimal
    return parseInt(extractedBits, 2);
}

async function interpretConvScale(RawData, DataType, PresentationsKey){
    //Die Textabelle erstmal in ein Objekt schreiben
    let ConversionScale = DataType.Presentations[PresentationsKey[0]].Scales;
    //Durch die Textabelle iterieren
    for (const scaleKey of Object.keys(ConversionScale)) {
        const scale = ConversionScale[scaleKey];
        let ScaleName=scaleKey;
        let ScaleUpBound=scale["Up Bound"];
        let ScaleLowBound=scale["Low Bound"];

        // Wenn RawData innerhalb der Grenzen liegt, dann ist es der Wert
        if (RawData >= ScaleLowBound && RawData <= ScaleUpBound) {
            return ScaleName;
        }
    }
    // Wenn kein Wert gefunden wurde, dann ist der Wert unbekannt
    return "Unknown";

}

async function interpretConvRaw(RawData, DataType, PresentationsKey){
    
}
