caredID = null;

function selectButton(btid) {
    document.getElementById('menu').hidden = true;
    caredID = btid;
    if (btid < 0) {
        document.getElementById('chunks').hidden = false;
    } else {
        lamp = document.getElementById('lamp').hidden = false;
    }
    var elem = document.body; // Make the body go full screen.
    requestFullScreen(elem);
};

function requestFullScreen(element) {
    // Supports most browsers and their versions.
    var requestMethod = element.requestFullScreen || element.webkitRequestFullScreen || element.mozRequestFullScreen || element.msRequestFullScreen;

    if (requestMethod) { // Native full screen.
        requestMethod.call(element);
    } else if (typeof window.ActiveXObject !== "undefined") { // Older IE.
        var wscript = new ActiveXObject("WScript.Shell");
        if (wscript !== null) {
            wscript.SendKeys("{F11}");
        }
    }
}