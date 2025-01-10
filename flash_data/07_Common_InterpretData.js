async function interpretData(response, DataType) {
    let responseWithoutIdentifier;
    // Entferne die ersten 4 Byte (den Identifier), wenn IDlength ==29
    if (globalConfigJSON.CAN_Bus.ID_length_bit == 29) {
    responseWithoutIdentifier = response.substring(8);
    }


    // Da der Datentyp Name immer unterschiedlich ist, müssen wir diesen erstmal ermitteln
    const PresentationsKey = Object.keys(DataType.Presentations);
    // Wir müssen die Response erstmal in eine dezimale Zahl wandeln
    let responseDec = BigInt('0x' + responseWithoutIdentifier);
    let responseHexString=responseWithoutIdentifier;
    // Nun können wir die Response interpretieren
    // Zuerst müssen wir den Wert extrahieren, an der richtigen Stelle von der Response
        //Achtung: Manchmal hat DataType.Presentations ein anderes Format, in dem keine Bit Pos und Bit Length angegeben sind.
        //In diesem Falle wird:
            //  Bit Pos = 0 gesetzt
            // Bit Length = StandardLength*8 gesetzt
    //Falls Bit Pos und Bit Length nicht vorhanden sind
    let RawData;
    let RawDataHexString;
    if ((!DataType.Presentations[PresentationsKey[0]].hasOwnProperty("Bit Pos")) && (!DataType.Presentations[PresentationsKey[0]].hasOwnProperty("Bit Length"))) {
        RawData = await extractValue(responseDec, parseInt(DataType.Presentations[PresentationsKey[0]]["Byte Pos"],10), 0, parseInt(DataType.Presentations[PresentationsKey[0]]["Standard Length"],10)*8);
        RawDataHexString = await extractValueHexString(responseHexString, parseInt(DataType.Presentations[PresentationsKey[0]]["Byte Pos"],10), 0, parseInt(DataType.Presentations[PresentationsKey[0]]["Standard Length"],10)*8);
    //Falls Bit Pos und Bit Length vorhanden sind
    }else{
        RawData = await extractValue(responseDec, parseInt(DataType.Presentations[PresentationsKey[0]]["Byte Pos"],10), parseInt(DataType.Presentations[PresentationsKey[0]]["Bit Pos"],10), parseInt(DataType.Presentations[PresentationsKey[0]]["Bit Length"],10));
        RawDataHexString = await extractValueHexString(responseHexString, parseInt(DataType.Presentations[PresentationsKey[0]]["Byte Pos"],10), 0, parseInt(DataType.Presentations[PresentationsKey[0]]["Bit Length"],10));
    }
    let ProcessedData;

    // Verschiedene Datentypen benötigen verschiedene Interpretationen
    // Texttable (ConversionScale)
    if ((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_SCALE")) {
        ProcessedData = await interpretConvScale (RawData, DataType, PresentationsKey);
    }
    // Blanke Dezimal-Zahl (ConversionRaw)
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_RAW")) {
        ProcessedData = RawData.toString();
    }
    // ASCII (ConversionASCII)
    else if((DataType.Presentations[PresentationsKey[0]].Convselector).includes("CONVERSION_ASCII")) {
        //Fall, dass RawData=0 ist
        if(RawData==0){
            ProcessedData= "No Data";
        }else{
            ProcessedData=await hexStringToAscii(RawDataHexString);
        }
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
    //Zusätzliche Bedingung: Bitlength ist durch 8 teilbar, und somit werden nur ganz Bytes extrahiert
    if ((bitLength >8)&&(bitLength%8==0)) {
        const startBit = bytePosition*8; //Bitposition muss nicht addiert werden, da eh immer =0 wenn bitLength%8==0
        const endBit = startBit + bitLength;
        const extractedBits = binaryString.slice(startBit-1, endBit-1); 
        return parseInt(extractedBits, 2);
    //Nächster Spezialfall: Standard Length=0, da Länge des Datums variabel. In diesem Fall einfach Daten bis zum Ende von decimal Number extrahieren
    }else if (bitLength==0) {
        const startBit = bytePosition*8; //Bitposition muss nicht addiert werden, da eh immer =0 wenn bitLength%8==0
        const extractedBits = binaryString.slice(startBit-1); 
        return parseInt(extractedBits, 2);

    }else{
  
        const startBit = bytePosition *8 + (8-bitPosition); // Berechne den Startbit-Index
        const endBit = startBit - bitLength; // Endbit-Index
        // Extrahiere die relevanten Bits
        const extractedBits = binaryString.slice(endBit-1, startBit-1);   
        // Konvertiere die extrahierten Bits zurück in Dezimal
        return parseInt(extractedBits, 2);
    }
}

async function extractValueHexString(HexString, bytePosition, bitPosition, bitLength) {
    if ((bitLength !=0)&&(bitLength%8==0)) {
        const startString = bytePosition*2; //Bitposition muss nicht addiert werden, da eh immer =0 wenn bitLength%8==0
        const endString = startString + (bitLength/4);
        const extractedHexString = HexString.slice(startString, endString);
        if (extractedHexString==""){
            return "5858"
        }
        return extractedHexString;
    //Nächster Spezialfall: Standard Length=0, da Länge des Datums variabel. In diesem Fall einfach Daten bis zum Ende von decimal Number extrahieren
    }else if (bitLength==0) {
        const startString = bytePosition*2; //Bitposition muss nicht addiert werden, da eh immer =0 wenn bitLength%8==0
        const extractedHexString = HexString.slice(startString-1);
        if (extractedHexString==""){
            return "5858"
        }
        return extractedHexString; 
    }
}



async function interpretConvScale(RawData, DataType, PresentationsKey){
    //Die Scale erstmal in ein Objekt schreiben
    let ConversionScale = DataType.Presentations[PresentationsKey[0]].Scales;
    //Durch die Scale iterieren
    // Scales mit "<n/a>"//////////////
    //Anderes Vorgehen für Scales, die "<n/a>" enthalten
    //Diese Scales sind nicht nur dazu da, Text auszugeben, sondern den Rohwert umzurechnen mit Offset und Faktor
    if (ConversionScale.hasOwnProperty("<n/a>")) {
        for (const scaleKey of Object.keys(ConversionScale)) {
            //Erstmal alle Daten in Variablen schreiben
            const scale = ConversionScale[scaleKey];
            let ScaleName=scaleKey;
            let ScaleUpBound=scale["Up Bound"];
            let ScaleLowBound=scale["Low Bound"];

            //Fall 1: 
                // Scale hat die Objekte "Offset"&"Factor"
                if (scale.hasOwnProperty("Offset")&&scale.hasOwnProperty("Factor")) {
                    //Erstmal alle Daten in Variablen schreiben
                    let ScaleOffset=parseInt(scale["Offset"],10);
                    let ScaleFactor=parseInt(scale["Factor"],10);
                    //Fall 1.1:
                        //Der Wert ist größer als Low Bound, und kleiner als Up Bound --> Dann Wert umerechnen
                    if (RawData >= ScaleLowBound && RawData <= ScaleUpBound) {
                        return (RawData*ScaleFactor)+ScaleOffset;
                    }
            //Fall 2:
                // Scale hat nicht die Objekte "Offset"&"Factor"
                }else{
                // Wenn RawData innerhalb der Grenzen liegt, dann gibt es keine Umrechnung und der Textwert wird zurückgegeben
                    if (RawData >= ScaleLowBound && RawData <= ScaleUpBound) {
                        return ScaleName;
                    }
                }
        }

    // Scales mit "<n/a>"//////////////
    }else{
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

    }

    // Wenn kein Wert gefunden wurde, dann ist der Wert unbekannt
    return "Unknown";

}

async function interpretConvRaw(RawData, DataType, PresentationsKey){
    
}

async function decimalToAscii(decimalNumber) {
    // Wandelt die Dezimalzahl in einen Array von Bytes um
    const bytes = [];
    let remaining = decimalNumber;

    // Solange die Zahl größer als 0 ist, extrahieren wir Bytes
    while (remaining > 0) {
        bytes.unshift(remaining % 256);  // Holen wir das niedrigste Byte (Modulo 256)
        remaining = Math.floor(remaining / 256);  // Reduzieren wir die Zahl um 256
    }

    // Wandelt die extrahierten Bytes in ASCII-Zeichen um
    return bytes
        .map(byte => (byte >= 32 && byte <= 126) ? String.fromCharCode(byte) : '')  // Nur druckbare ASCII-Zeichen
        .join('');
}

async function hexStringToAscii(hexString) {
    // Entfernt Leerzeichen und sorgt für gerade Länge
    hexString = hexString.replace(/\s+/g, '').padStart(hexString.length + (hexString.length % 2), '0');
    // Zerlegt den Hex-String in Paare und wandelt jedes Byte in ein ASCII-Zeichen um
    return hexString.match(/.{2}/g)  // Teilt den Hex-String in Paare
        .map(byte => String.fromCharCode(parseInt(byte, 16)))  // Wandelt jedes Byte in ASCII um
        .join('');  // Verbindet die ASCII-Zeichen zu einem String
}