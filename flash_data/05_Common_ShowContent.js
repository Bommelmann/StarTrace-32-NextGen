// Function to show selected content and hide others
async function showContent(section) {
    const sections = document.querySelectorAll('.content-section');
    sections.forEach(sec => sec.classList.add('hidden'));
    document.getElementById(section).classList.remove('hidden');
}



// Function to show selected tab and set the tab button to active
async function showTab(tab) {
    const tabs = document.querySelectorAll('.tabcontent');
    tabs.forEach(tab => tab.style.display = 'none');
    document.getElementById(tab).style.display = 'block';

    const tablinks = document.getElementsByClassName("tablink");
    for (let i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    const activeTabButton = document.querySelector(`.tablink[onclick*="${tab}"]`);
    if (activeTabButton) {
        activeTabButton.className += " active";
    }
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

// Hilfsfunktion für Verzögerung
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

    // Funktion zum Erstellen neuer Tabs
async function createTab(tabName) {
    const tabsContainer = document.getElementById('tabs-container');

    // Neuen Tab-Button erstellen
    const newTabButton = document.createElement('button');
    newTabButton.className = 'tablink';
    newTabButton.textContent = tabName;
    newTabButton.onclick = function(event) {
        openTab(event, tabName);
    };

    // Neuen Tab-Inhalt erstellen
    const newTabContent = document.createElement('div');
    newTabContent.id = tabName;
    newTabContent.className = 'tabcontent';
    newTabContent.innerHTML = `
    <div id="ECUs" class="tabcontent">
        <h1>Detected ECUs</h1>
        <div id="json-tree"></div>
        <table>
            <tr>
                <th>ECU Name</th>
                <th>Diagnostic Version</th>
            </tr>
            <tbody>
                <!-- Zeilen werden hier dynamisch hinzugefügt -->
            </tbody>

        </table>
    </div>
    `;

    // Tab-Button und Tab-Inhalt hinzufügen
    tabsContainer.appendChild(newTabButton);
    document.body.appendChild(newTabContent);
}