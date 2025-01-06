
async function ScanECUs() {
    console.log("ScanECUs");
    let request;
    // Iteriere durch ECUs
    for (const device of globalConfigJSON.ECUs) {

        if (device.AutomaticConnect==true){
            console.log(`Device: ${device.shortLabel}`);
            
            // TA: Tester Address
            let TA = globalConfigJSON.CAN_Bus.testerAddress.toString(16).toUpperCase();
            TA = TA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2
            
            // SA: Source Address
            let SA = device.SA.toString(16).toUpperCase();
            SA = SA.padStart(2, '0'); // Fügt eine führende 0 hinzu, wenn Länge < 2
            
            // DiagVersionService und ComService
            let DiagVersionService = device.DiagVersionService;
            let ComService = DiagVersionService.ComService;
            
            if(globalConfigJSON.CAN_Bus.ID_length_bit==29){
            // Erstelle request
            request = "18DA" + SA + TA + ComService;
            console.log ("request:", request);
            }
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
