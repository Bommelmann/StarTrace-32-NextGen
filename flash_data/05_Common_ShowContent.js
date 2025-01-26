// Function to show selected content and hide others
async function showContent(sectionId) {
    //Wenn der erste Tab noch nicht existiert, dann wird ein ErrorModal Wait gezeigt
    if (document.querySelector(`#${sectionId} .tablink`) === null) {
        showErrorModalWait('No data available yet. Please wait for ' + sectionId + ' data to load.', sectionId);
    }else{
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

async function handleTabButtonDropDownClick(DropDownText,parentTabName){

    //Schritt 1: Hole parentTabName
    let parentTab = document.getElementById(parentTabName);
    //Schritt 2: alle Elemente innerhalb dieses Tabs holen
    let parentTabelements = parentTab.querySelectorAll('*');
    //Schritt 3: Durch die elemente iterieren
    // Schritt 3: Durchlaufe die Elemente und suche nach dem Text
    for (let i = 0; i < parentTabelements.length; i++) {
        parentTabelements[i].classList.remove('highlight');
        }
    
    for (let i = 0; i < parentTabelements.length; i++) {
        parentTabelements[i].classList.remove('highlight');
        if (parentTabelements[i].textContent==DropDownText) {
            // Schritt 4: Markiere das Element, wenn der Text gefunden wird
            parentTabelements[i].classList.add('highlight');
            
            // Schritt 5: Scrolle zum gefundenen Element
            parentTabelements[i].scrollIntoView({
                behavior: 'smooth',
                block: 'center'  // 
            });
            break;
        }
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

// Funktion zum Anzeigen des Wait Modals
function showErrorModalWait(message,sectionId) {
    const errorModalWait = document.getElementById(`error-modal-wait-${sectionId}`);
    const errorTextWait = document.getElementById(`error-text-wait-${sectionId}`);
    errorTextWait.innerText = message;
    errorModalWait.style.display = 'flex'; // Modal wird sichtbar
}

// Event listener for closing the light error modal
const closeBtnLight = document.getElementById('close-btn-light');
closeBtnLight.addEventListener('click', function() {
    const errorModalLight = document.getElementById('error-modal-light');
    errorModalLight.style.display = 'none'; // Modal schließen
});

// Event listener for closing the wait modal
const closeBtnWaitidentification = document.getElementById('close-btn-wait-identification');
closeBtnWaitidentification.addEventListener('click', function() {
    const errorModalWait = document.getElementById('error-modal-wait-identification');
    errorModalWait.style.display = 'none'; // Modal schließen
});

// Event listener for closing the wait modal
const closeBtnWaitmeasurement= document.getElementById('close-btn-wait-measurement');
closeBtnWaitmeasurement.addEventListener('click', function() {
    const errorModalWait = document.getElementById('error-modal-wait-measurement');
    errorModalWait.style.display = 'none'; // Modal schließen
});

// Event listener for closing the wait modal
const closeBtnWait = document.getElementById('close-btn-wait-faultcodes');
closeBtnWait.addEventListener('click', function() {
    const errorModalWait = document.getElementById('error-modal-wait-faultcodes');
    errorModalWait.style.display = 'none'; // Modal schließen
});

// Hilfsfunktion für Verzögerung
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}





async function ShowLoadingIcon(ID){
document.getElementById(ID).style.display = 'block';
}

async function UnshowLoadingIcon(ID){
    document.getElementById(ID).style.display = 'none';
}
    
