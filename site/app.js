const askButton = document.querySelector('#ask-button');
const askMessage = document.querySelector('#ask-message');
const askedAtElement = document.querySelector('#asked-at');
const answeredAtElement = document.querySelector('#answered-at');

let waitingForAnswer = false;
let answerMessage = '';
let askedAt = null;
let answeredAt = null;
let statusRequestInFlight = false;

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
  if (statusRequestInFlight) return;
  statusRequestInFlight = true;

  try {
    const response = await fetch('/api/status', { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    waitingForAnswer = data.state === 'waiting';
    answerMessage = data.state === 'answered' ? '元気！' : '';
    askedAt = data.askedAt;
    answeredAt = data.answeredAt;
    updateUi();
  } catch (error) {
    askMessage.textContent = 'ESP32の状態を取得できません。';
  } finally {
    statusRequestInFlight = false;
  }
}

async function askEsp32() {
  if (waitingForAnswer) return;

  answerMessage = '';
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

askButton.addEventListener('click', askEsp32);
updateUi();
loadStatus();
setInterval(loadStatus, 1000);
