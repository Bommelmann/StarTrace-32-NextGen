// Funktion zum Erstellen neuer Tabs
async function createTabButton(parentcontentName, tabcontentName, DiagnosticClass) {
    // Get Parent Container (Tabs-Containter-Measurement/Identification...)
    const tabsContainer = document.getElementById(parentcontentName);

    // Neuen Dropdown Container erstellen
    const newDropDown = document.createElement('div');
    newDropDown.className = 'dropdown';

    // Neuen Tab-Button erstellen
    const newTabButton = document.createElement('button');
    newTabButton.className = 'tablink';
    newTabButton.textContent = 'Electronic Control Unit: '+tabcontentName;
    newTabButton.setAttribute('onclick', `showTab('${tabcontentName+DiagnosticClass}')`);

    // Neuen Drop Down Content erstellen
    const newDropDownContent = document.createElement('div');
    newDropDownContent.className = 'dropdown-content';
    // Spezifische ID, damit es nancher leichter zu finden ist
    newDropDownContent.id = 'dropdown-content'+tabcontentName+DiagnosticClass;

    // Tab-Button unterhalb des DropDowns einfügen
    newDropDown.appendChild(newTabButton);
    // DropDown Content unterhalb des Dropdowns einfügen
    newDropDown.appendChild(newDropDownContent);
    // Dropdown Element, das jetzt auch den Tab-Button enthält, in den Tabs-Container einfügen
    tabsContainer.appendChild(newDropDown);    
    
    // Ensure all buttons are equally wide
    const tabButtons = tabsContainer.querySelectorAll('.dropdown');
    const buttonWidth = 100 / tabButtons.length;
    tabButtons.forEach(button => {
        button.style.width = `${buttonWidth}%`;
    });

}

async function createTabButtonDropDown(DropDownContentParentName, DropDown){
    // Get Tab Button Parent to create child
    const DropDownContentParent = document.getElementById(DropDownContentParentName);
    //Get DropDownContenParentName without "dropdown-content", because this is the name of the parent tab
    const parentTabName = DropDownContentParentName.replace('dropdown-content','');
    // Jetzt die Elemente des Dropdowns erstellen
    // Element vom Typ a ist so ein menübutton
    const a = document.createElement("a");
    // Textcontent ist Datentypname
    a.textContent = DropDown;
    // href ist eigentlich eine URL, wird aber hier nicht benötigt
    a.href = "#";
    a.addEventListener("click", function(event) {
        event.preventDefault(); // Verhindert das Laden der Seite
        handleTabButtonDropDownClick(DropDown,parentTabName);
    });
    DropDownContentParent.appendChild(a);

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
        table.style.tableLayout = 'fixed'; // Ensure consistent table layout
        content.appendChild(table);
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