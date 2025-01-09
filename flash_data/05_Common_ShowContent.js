// Function to show selected content and hide others
async function showContent(sectionId) {
    const sections = document.querySelectorAll('.content-section');
    //Andere Contents verstecken
    sections.forEach(section => section.classList.add('hidden'));
    //SectionId Content anzeigen
    document.getElementById(sectionId).classList.remove('hidden');
    //Ersten Tab anzeigen
    document.querySelector(`#${sectionId} .tablink`).click();

    // Set button color to match content background
    const buttons = document.querySelectorAll('.menu ul li a');
    buttons.forEach(button => button.style.backgroundColor = ''); // Reset all buttons
    const activeButton = document.querySelector(`.menu ul li a[onclick="showContent('${sectionId}')"]`);
    if (activeButton) {
        activeButton.style.backgroundColor = "#575757";
    }
}



// Function to show selected tab and set the tab button to active
async function showTab(tab) {
    // Hide all tabs
    const tabs = document.querySelectorAll('.tabcontent');
    tabs.forEach(tab => tab.style.display = 'none');
    // Show selected tab
    let tmptab=document.getElementById(tab);
    tmptab.style.display = 'block';
    // Show all children of the tab
    showAllChildren(tmptab);
    // Remove active class from all tab buttons
    const tablinks = document.getElementsByClassName("tablink");
    for (let i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    // Set the tab button to active
    const activeTabButton = document.querySelector(`.tablink[onclick*="${tab}"]`);
    if (activeTabButton) {
        activeTabButton.className += " active";
    }
}

async function showAllChildren(element) {
    // Stelle sicher, dass das Element existiert
    if (!element) return;
      //Damit Tabellen richtig angzeigt werden, dürfen nicht alle Kinder sichtbar gemacht werden
      if(element.tagName !== 'TABLE' && element.tagName !== 'TH' && element.tagName !== 'TR' && element.tagName !== 'TBODY' && element.tagName !== 'TD' && element.tagName !== 'SPAN'){
            // Setze das Element selbst sichtbar
            element.style.display = 'block';
        }
    
    // Iteriere durch alle direkten Kinder
    Array.from(element.children).forEach(child => {
      // Rufe die Funktion für jedes Kind erneut auf
      showAllChildren(child);
    });
  }
  



async function openTab(evt, tabName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablink");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    evt.currentTarget.className += " active";
}


    // Funktion, die eine Promise zurückgibt, die aufgelöst wird, wenn der Button geklickt wird
    function waitForDisclaimer() {
        return new Promise((resolve) => {
            const disclaimer_continue = document.getElementById('disclaimer-continue');
            disclaimer_continue.addEventListener('click', function () {
                resolve();
            });
        });
    }


// Funktion zum Anzeigen des Modals
function showErrorModal(error) {
    const errorModal = document.getElementById('error-modal');
    const errorText = document.getElementById('error-text');
    errorText.innerText=error;
    errorModal.style.display = 'flex'; // Modal wird sichtbar
}

// Funktion zum Anzeigen des leichten Modals
function showErrorModalLight(error) {
    const errorModalLight = document.getElementById('error-modal-light');
    const errorTextLight = document.getElementById('error-text-light');
    errorTextLight.innerText = error;
    errorModalLight.style.display = 'flex'; // Modal wird sichtbar
}

// Event listener for closing the light error modal
const closeBtnLight = document.getElementById('close-btn-light');
closeBtnLight.addEventListener('click', function() {
    const errorModalLight = document.getElementById('error-modal-light');
    errorModalLight.style.display = 'none'; // Modal schließen
});

// Hilfsfunktion für Verzögerung
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

    // Funktion zum Erstellen neuer Tabs
async function createTabButton(parentcontentName, tabcontentName) {
    // Get Parent Container
    const tabsContainer = document.getElementById(parentcontentName);

    // Neuen Tab-Button erstellen
    const newTabButton = document.createElement('button');
    newTabButton.className = 'tablink';
    newTabButton.textContent = tabcontentName;
    newTabButton.setAttribute('onclick', `showTab('${tabcontentName}')`);

    // Tab-Button hinzufügen
    tabsContainer.appendChild(newTabButton);

    // Add fixed position to the tab buttons
    tabsContainer.style.position = 'fixed';
    tabsContainer.style.top = '0';
    tabsContainer.style.width = 'calc(100% - 250px)'; // Adjust width to fill screen from left menu to right edge
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
    childContent.id = childcontentName;
    childContent.className = 'tabcontent';
    childContent.innerHTML = `
    <div id=${childcontentName} class="tabcontent">
    <div>
    `;
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
    cellValue.textContent = childContent.Result;
    row.appendChild(cellValue);

    // Append the row to the table
    table.appendChild(row);
}