const ledButton = document.querySelector('#led-toggle');
const ledMessage = document.querySelector('#led-message');
let ledBlinking = false;

function updateLedUi() {
  ledButton.disabled = ledBlinking;
  ledButton.textContent = ledBlinking ? '点滅中' : '点滅';
  ledMessage.textContent = ledBlinking
    ? 'LEDが3回点滅しています。'
    : 'ボタンを押すとLEDが3回点滅します。';
}

async function refreshLed() {
  try {
    const response = await fetch('/api/led', { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    ledBlinking = Boolean(data.blinking);
    updateLedUi();
  } catch (error) {
    ledButton.textContent = '操作できません';
    ledMessage.textContent = 'LEDの状態を取得できません。';
  }
}

async function toggleLed() {
  ledButton.disabled = true;
  try {
    const response = await fetch('/api/led', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: new URLSearchParams({ state: 'blink' }),
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    ledBlinking = Boolean(data.blinking);
    updateLedUi();
  } catch (error) {
    ledMessage.textContent = 'LEDを操作できません。';
    ledButton.disabled = false;
  } finally {
    if (!ledBlinking) ledButton.disabled = false;
  }
}

ledButton.addEventListener('click', toggleLed);

refreshLed();
setInterval(refreshLed, 300);
