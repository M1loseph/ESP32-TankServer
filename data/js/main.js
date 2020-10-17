import { sendWS } from './websocket.js';
import { getGamepadInfo, processGamepad } from './gamepad_processing.js';
import { COMMANDS } from './tank_commands.js';
import { DEFAULT_CONFIG } from './configs.js';

// checking gamepad info every few milliseconds
const GAMEPAD_INTERVAL = 33;
const MILLIS = ' [ms]';

setInterval(function () {
    let gamepadState = getGamepadInfo();
    if (gamepadState) {
        let messages;
        let functions;

        [messages, functions] = processGamepad(gamepadState, DEFAULT_CONFIG);
        functions.forEach(f => f());
        messages.forEach(m => {
            sendWS(m);
        });
    }
}, GAMEPAD_INTERVAL);

Array.from(document.getElementsByClassName('dropdown-button')).forEach(sidebarElement => {
    sidebarElement.onclick = function (event) {
        let dropdownContent = this.nextElementSibling;
        dropdownContent.classList.toggle('hidden-dropdown-container');
    }
});

let gamepadImage = document.getElementById('gamepad-image');
let sidebar = document.getElementById('sidebar');
let welcomeTitle = document.getElementById('welcome-title');
let modal = document.getElementById('modal');

// elements from LED color picker dialog
let colorDialog = document.getElementById('color-picker-dialog');
let ledColorPicker = document.getElementById('led-color-picker');

// elements from dialog for setting speed
let speedDialog = document.getElementById('speed-dialog');
let speedSlider = document.getElementById('speed-slider');
let speedLabel = document.getElementById('speed-slider-label');
speedLabel.textContent = speedSlider.value;
speedSlider.onchange = () => speedLabel.textContent = speedSlider.value;

// elements from volume dialog
let volumeDialog = document.getElementById('volume-dialog');
let volumeSlider = document.getElementById('volume-slider');
let volumeLabel = document.getElementById('volume-slider-label');
volumeLabel.textContent = volumeSlider.value;
volumeSlider.onchange = () => volumeLabel.textContent = volumeSlider.value;

// elements from brightness dialog
let brightnessDialog = document.getElementById('brightness-dialog');
let brightnessSlider = document.getElementById('brightness-slider');
let brightnessLabel = document.getElementById('brightness-slider-label');
brightnessLabel.textContent = brightnessSlider.value;
brightnessSlider.onchange = () => brightnessLabel.textContent = brightnessSlider.value;

// elements from interval dialog
let intervalDialog = document.getElementById('interval-dialog');
let intervalSlider = document.getElementById('interval-slider');
let intervalLabel = document.getElementById('interval-slider-label');
intervalLabel.textContent = intervalSlider.value + MILLIS;
intervalSlider.onchange = () => intervalLabel.textContent = intervalSlider.value + MILLIS;

function closeSidebar(event) {
    console.log('trying to close the sidebar...');
    if (event.target === gamepadImage || event.target === welcomeTitle) {
        sidebar.classList.add('hidden-sidebar');
        console.log('closing sidebar...');
    }
}

function openSidebar() {
    sidebar.classList.remove('hidden-sidebar');
    console.log('opening sidebar...');
}

function openColorPickerDialog() {
    console.log('opening color picker...');
    colorDialog.classList.remove('hidden-dialog');
    modal.classList.remove('hidden-modal');
}

function openSpeedDialog() {
    console.log('opening speed dialog...');
    speedDialog.classList.remove('hidden-dialog');
    modal.classList.remove('hidden-modal');
}

function openVolumeDialog() {
    console.log('opening volume dialog...');
    volumeDialog.classList.remove('hidden-dialog');
    modal.classList.remove('hidden-modal');
}

function openBrightnessDialog() {
    console.log('opening brightness dialog...');
    brightnessDialog.classList.remove('hidden-dialog');
    modal.classList.remove('hidden-modal');
}

function openIntervalDialog() {
    console.log('opening brightness dialog...');
    intervalDialog.classList.remove('hidden-dialog');
    modal.classList.remove('hidden-modal');
}

function closeDialogs() {
    console.log('closing all dialogs...');
    modal.classList.add('hidden-modal');
    brightnessDialog.classList.add('hidden-dialog');
    volumeDialog.classList.add('hidden-dialog');
    colorDialog.classList.add('hidden-dialog');
    intervalDialog.classList.add('hidden-dialog');
    speedDialog.classList.add('hidden-dialog');
}

function sendColor() {
    let hexColor = ledColorPicker.value;
    // remove 1st char
    hexColor = hexColor.substring(1);

    let RGBHex = hexColor.match(/.{1,2}/g);
    let RGB = [
        parseInt(RGBHex[0], 16),
        parseInt(RGBHex[1], 16),
        parseInt(RGBHex[2], 16),
    ];
    sendWS(COMMANDS.LED.SET_CUSTOM_COLOR + ' ' + RGB[0] + ' ' + RGB[1] + ' ' + RGB[2]);
}

function sendSpeed() {
    let message = COMMANDS.ENGINES.SPEED + ' ' + speedSlider.value;
    sendWS(message);
}

function sendVolume() {
    let message = COMMANDS.MP3.SET_VOLUME + ' ' + volumeSlider.value;
    sendWS(message);
}

function sendBrightness() {
    let message = COMMANDS.LED.SET_BRIGHTNESS + ' ' + brightnessSlider.value;
    sendWS(message);
}

function sendInterval() {
    let message = COMMANDS.LED.SET_ANIMATION_INTERVAL + ' ' + intervalSlider.value;
    sendWS(message);
}

// export it to html
window.sendWS = sendWS;
window.closeSidebar = closeSidebar;
window.openSidebar = openSidebar;
window.openColorPickerDialog = openColorPickerDialog;
window.openSpeedDialog = openSpeedDialog;
window.openBrightnessDialog = openBrightnessDialog;
window.openIntervalDialog = openIntervalDialog;
window.closeDialogs = closeDialogs;
window.openVolumeDialog = openVolumeDialog;

window.sendColor = sendColor;
window.sendSpeed = sendSpeed;
window.sendVolume = sendVolume;
window.sendBrightness = sendBrightness;
window.sendInterval = sendInterval;

window.COMMANDS = COMMANDS;