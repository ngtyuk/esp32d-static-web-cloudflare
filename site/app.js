const askButton = document.querySelector('#ask-button');
const askMessage = document.querySelector('#ask-message');
const askedAtElement = document.querySelector('#asked-at');
const answeredAtElement = document.querySelector('#answered-at');

let waitingForAnswer = false;
let answerMessage = '';
let askedAt = localStorage.getItem('esp32d-asked-at');
let answeredAt = localStorage.getItem('esp32d-answered-at');

function formatDate(value) {
  return value ? new Date(value).toLocaleString('ja-JP') : '—';
}

function updateUi() {
  askButton.disabled = waitingForAnswer;
  askButton.textContent = waitingForAnswer ? '聞いています...' : '元気？';
  askMessage.textContent = waitingForAnswer
    ? '元気かどうか聞いています...'
    : answerMessage || 'ボタンを押してESP32に聞いてください。';
  askedAtElement.textContent = formatDate(askedAt);
  answeredAtElement.textContent = formatDate(answeredAt);
}

async function loadStatus() {
  try {
    const response = await fetch('/api/status', { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    waitingForAnswer = Boolean(data.waiting);
    updateUi();
  } catch (error) {
    askMessage.textContent = 'ESP32の状態を取得できません。';
  }
}

async function askEsp32() {
  if (waitingForAnswer) return;

  askedAt = new Date().toISOString();
  answeredAt = null;
  answerMessage = '';
  localStorage.setItem('esp32d-asked-at', askedAt);
  localStorage.removeItem('esp32d-answered-at');
  waitingForAnswer = true;
  updateUi();

  try {
    const response = await fetch('/api/ask', { method: 'POST', cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
  } catch (error) {
    waitingForAnswer = false;
    answerMessage = 'ESP32に質問できません。';
    updateUi();
  }
}

const events = new EventSource('/events');
events.addEventListener('answer', (event) => {
  const data = JSON.parse(event.data);
  waitingForAnswer = false;
  answerMessage = data.message || '元気！';
  answeredAt = new Date().toISOString();
  localStorage.setItem('esp32d-answered-at', answeredAt);
  updateUi();
});
events.onerror = () => {
  if (waitingForAnswer) askMessage.textContent = 'ESP32との接続を再試行しています...';
};

askButton.addEventListener('click', askEsp32);
updateUi();
loadStatus();
