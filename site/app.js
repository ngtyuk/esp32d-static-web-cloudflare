const fields = {
  hostname: document.querySelector('#hostname'),
  ip: document.querySelector('#ip'),
  uptime: document.querySelector('#uptime'),
  updated: document.querySelector('#updated'),
};
const badge = document.querySelector('#health-badge');
const message = document.querySelector('#health-message');
const ledButton = document.querySelector('#led-toggle');
const ledMessage = document.querySelector('#led-message');
let ledOn = false;

function formatUptime(seconds) {
  const total = Number(seconds || 0);
  const days = Math.floor(total / 86400);
  const hours = Math.floor((total % 86400) / 3600);
  const minutes = Math.floor((total % 3600) / 60);
  const secs = total % 60;
  return `${days}d ${hours}h ${minutes}m ${secs}s`;
}

async function refreshHealth() {
  try {
    const response = await fetch('/api/health', { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    fields.hostname.textContent = data.hostname || 'ESP-32D';
    fields.ip.textContent = data.ip || '—';
    fields.uptime.textContent = formatUptime(data.uptime);
    fields.updated.textContent = new Date().toLocaleTimeString('ja-JP');
    badge.textContent = '正常';
    message.textContent = '正常に取得しました。';
  } catch (error) {
    badge.textContent = '取得失敗';
    message.textContent = 'デバイスの状態を取得できません。';
  }
}

function updateLedUi() {
  ledButton.textContent = ledOn ? '消灯' : '点灯';
  ledMessage.textContent = ledOn ? '点灯中' : '消灯中';
}

async function refreshLed() {
  try {
    const response = await fetch('/api/led', { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    ledOn = Boolean(data.on);
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
      body: new URLSearchParams({ state: ledOn ? 'off' : 'on' }),
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    ledOn = Boolean(data.on);
    updateLedUi();
  } catch (error) {
    ledMessage.textContent = 'LEDを操作できません。';
  } finally {
    ledButton.disabled = false;
  }
}

ledButton.addEventListener('click', toggleLed);

refreshHealth();
refreshLed();
setInterval(refreshHealth, 10000);
setInterval(refreshLed, 10000);
