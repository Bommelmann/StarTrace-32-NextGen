
async function loadHomeContent() {
    const response = await getAsset('home.html', 'text/html');
    const homeContent = await response.text();
    document.getElementById('home').innerHTML = homeContent;
}