    // Funktion zum Erstellen neuer Tabs
    async function createTabButton(parentcontentName, tabcontentName, DiagnosticClass) {
        // Get Parent Container
        const tabsContainer = document.getElementById(parentcontentName);
    
        // Neuen Tab-Button erstellen
        const newTabButton = document.createElement('button');
        newTabButton.className = 'tablink';
        newTabButton.textContent = tabcontentName;
        newTabButton.setAttribute('onclick', `showTab('${tabcontentName+DiagnosticClass}')`);
    
        // Tab-Button hinzufügen
        tabsContainer.appendChild(newTabButton);
    
        // Add fixed position to the tab buttons
        tabsContainer.style.zIndex = '1000';
    
        // Ensure all buttons are equally wide
        const tabButtons = tabsContainer.querySelectorAll('.tablink');
        const buttonWidth = 100 / tabButtons.length;
        tabButtons.forEach(button => {
            button.style.width = `${buttonWidth}%`;
        });
    }
    
    
    async function createContent(parentcontentName, childcontentName){
        // Get Parent Container
        const content = document.getElementById(parentcontentName);
        if (!content) {
            console.error(`Fehler: Content mit ID ${parentcontentName} nicht gefunden`);
            return;
        }
        // Neuen Tab-Inhalt erstellen
        const childContent = document.createElement('div');
        childContent.id = childcontentName+parentcontentName;
        childContent.className = 'tabcontent';
        //Dem übergebenen tabName the TabContent anhängen
        content.appendChild(childContent);
    
    }
    
    async function createHeading(parentContentName, headingcontentName){
        // Get Parent Container
        const content = document.getElementById(parentContentName);
        if (!content) {
            console.error(`Fehler: Content mit ID ${parentContentName} nicht gefunden`);
            return;
        }
        // Neuen Tab-Inhalt erstellen
        const headingContent = document.createElement('div');
        headingContent.id = headingcontentName;
        headingContent.className = 'tabcontent';
        headingContent.innerHTML = `
        <div id=${headingcontentName} class="tabcontent">
            <h1>${headingcontentName}</h1>
        `;
        //Dem übergebenen tabName the TabContent anhängen
        content.appendChild(headingContent);
    
    }
    
    async function createDataEntry(parentContentName, childContent){
        // Get Parent Container
        const content = document.getElementById(parentContentName);
        if (!content) {
            console.error(`Fehler: Content mit ID ${parentContentName} nicht gefunden`);
            return;
        }
    
        // Check if table exists, if not create one
        let table = content.querySelector('table');
        if (!table) {
            table = document.createElement('table');
            table.style.width = '100%';
            content.appendChild(table);
    
            // Set table layout to auto for auto-sizing columns
            table.style.tableLayout = 'auto';
        }
    
        // Create a new row
        let row = document.createElement("tr");
    
        // Create the left cell with bold text
        let cellKey = document.createElement("td");
        cellKey.textContent = childContent.DataName;
        cellKey.style.fontWeight = 'bold';
        row.appendChild(cellKey);
    
        // Create the right cell
        let cellValue = document.createElement("td");
        // Insert the payload data together with the unit, if existing
        const PresentationsKey = Object.keys(childContent.Presentations);
        if(childContent.Presentations[PresentationsKey[0]].hasOwnProperty("Unit")){
            if(childContent.Presentations[PresentationsKey[0]].Unit != "year" && childContent.Presentations[PresentationsKey[0]].Unit != "day"){
                cellValue.textContent = childContent.Result+' ['+childContent.Presentations[PresentationsKey[0]].Unit+']';
            }
            
        }else{
            cellValue.textContent = childContent.Result;
        }
        row.appendChild(cellValue);
    
        // Append the row to the table
        table.appendChild(row);
    }