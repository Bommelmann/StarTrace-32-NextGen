
        // Function to display JSON tree
        function displayJsonTree(json, container) {


            
            container.innerHTML = ''; // Clear previous content
            //Stack mit 3 Eigenschaften
                //key: Startpunkt der JSON Struktur
                //value: json object
                //level: Hirarchielevel
            const stack = [{ key: 'root', value: json, level: 0 }];

            while (stack.length > 0) {
                //Oberste Element des Stacks wird entfernt und in die Variablen key, value und level eingefügt
                const { key, value, level } = stack.pop();
                //Item wird erstellt und der CSS Klasse json-tree-item zugewiesen
                const item = document.createElement('div');
                item.className = 'json-tree-item';
                //Item wird erstellt für Header
                const header = document.createElement('div');
                //2 CSS Klassen werden zugewiesen (auf einaml)
                header.className = 'json-tree-header json-tree-level-' + level;
                //Button wird erstellt mit + Zeichen und, und wird dem key zugewiesen
                header.innerHTML = `<button class="json-tree-button">+</button> ${key.replace(/_/g, ' ')}`;
                item.appendChild(header);

                const content = document.createElement('div');
                content.className = 'json-tree-content';
                content.textContent = value;
                
                item.appendChild(content);

                header.addEventListener('click', function() {
                    content.classList.toggle('active');
                    header.querySelector('.json-tree-button').textContent = content.classList.contains('active') ? '-' : '+';
                });
            }
        }

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
                    return response;
                    }   
                    }catch(error){
                        //Fehlerbehandlung   
                        console.error('Fehler bei der Anfrage:', error);
                        showErrorModal(error.message);
        
                    }
        
        }
        //Object to verify which ECU is available and if, with which diagnostic version


        
// Function to proceed to the Identification page
function proceedToIdentification() {
    // Get selected ECU values
    const selectedECUs = Array.from(document.querySelectorAll('input[name="ecu"]:checked'))
                                .map(input => input.value);
    
    if (selectedECUs.length === 0) {
        alert('Please select at least one ECU to continue.');
    } else {
        // Close ECU selection modal and navigate to "Identification" section
        document.getElementById('ecu-modal').style.display = 'none';
        showContent('identification');
    }
}


// Function to fetch JSON from server
function fetchJson() {
    fetch('/get-date')
        .then(response => response.json())
        .then(data => {
            document.getElementById('json-output').textContent = JSON.stringify(data);
        })
        .catch(error => {
            document.getElementById('json-output').textContent = 'Error fetching JSON';
            console.error('Error:', error);
        });
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