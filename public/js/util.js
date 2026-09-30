import { lang, t } from './i18n.js';

const ESC = { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' };
export function esc(s) {
  return String(s ?? '').replace(/[&<>"']/g, (c) => ESC[c]);
}

// A maximal run of Latin words (identifiers, "C++20", "Meyers' Singleton", "std::unique_ptr").
const LATIN_RUN = /[A-Za-z_][\w+#.:'’\-/&<>=*~]*(?:[ \t]+[A-Za-z0-9_][\w+#.:'’\-/&<>=*~]*)*/g;
const TRAILING_PUNCT = /[.:'’\-/]+$/;

/** Plain text → HTML; in Persian, Latin runs are bidi-isolated so "C++" or "std::vector" render correctly. */
function plain(s) {
  if (lang() !== 'fa') return esc(s);
  let out = '';
  let last = 0;
  for (const m of s.matchAll(LATIN_RUN)) {
    let core = m[0].replace(TRAILING_PUNCT, '');
    if (!core) continue;
    let start = m.index;
    // "(Type Erasure)" → isolate the parentheses too, so they never mirror or split.
    if (s[start - 1] === '(' && s[start + core.length] === ')') {
      start -= 1;
      core = `(${core})`;
    }
    const nw = core.length <= 32 ? ' class="nw"' : '';
    out += esc(s.slice(last, start)) + `<bdi dir="ltr"${nw}>${esc(core)}</bdi>`;
    last = start + core.length;
  }
  return out + esc(s.slice(last));
}

/** Renders content text: `code` → <code>, **bold** → <strong>, everything else escaped. */
export function rich(text) {
  return String(text ?? '')
    .split(/(`[^`]+`)/g)
    .map((part) => {
      if (part.length > 2 && part.startsWith('`') && part.endsWith('`')) {
        return `<code class="ic" dir="ltr">${esc(part.slice(1, -1))}</code>`;
      }
      return part
        .split(/(\*\*[^*]+\*\*)/g)
        .map((seg) => (seg.length > 4 && seg.startsWith('**') && seg.endsWith('**') ? `<strong>${plain(seg.slice(2, -2))}</strong>` : plain(seg)))
        .join('');
    })
    .join('');
}

export function icon(name, cls = '') {
  return `<svg class="icon ${cls}" aria-hidden="true" focusable="false"><use href="icons.svg#i-${name}"></use></svg>`;
}

export function difficultyDots(level) {
  return `<span class="diff" role="img" aria-label="${esc(t('diff.label'))}: ${esc(t(`diff.${level}`))}">${[1, 2, 3]
    .map((i) => `<i class="${i <= level ? 'on' : ''}"></i>`)
    .join('')}</span>`;
}

export const prefersReducedMotion = () => window.matchMedia('(prefers-reduced-motion: reduce)').matches;

// ------------------------------------------------------------------ toasts
export function toast(title, { sub = '', iconName = 'sparkles', tone = 'indigo', timeout = 3200 } = {}) {
  const host = document.getElementById('toasts');
  if (!host) return;
  const el = document.createElement('div');
  el.className = `toast tone-${tone}`;
  el.innerHTML = `<span class="t-icon">${icon(iconName)}</span><span>${esc(title)}${sub ? `<small>${esc(sub)}</small>` : ''}</span>`;
  host.appendChild(el);
  setTimeout(() => {
    el.classList.add('leaving');
    el.addEventListener('animationend', () => el.remove(), { once: true });
    setTimeout(() => el.remove(), 400);
  }, timeout);
}

// ------------------------------------------------------------------ confetti
export function confetti() {
  if (prefersReducedMotion()) return;
  const colors = ['#7C3AED', '#0E7490', '#DB2777', '#F59E0B', '#4F46E5', '#F97316', '#22C55E'];
  const host = document.createElement('div');
  host.className = 'confetti';
  host.setAttribute('aria-hidden', 'true');
  for (let i = 0; i < 70; i++) {
    const p = document.createElement('i');
    p.style.left = `${Math.random() * 100}%`;
    p.style.background = colors[i % colors.length];
    p.style.setProperty('--dx', `${(Math.random() - 0.5) * 240}px`);
    p.style.setProperty('--rot', `${Math.random() * 720 - 360}deg`);
    p.style.animationDelay = `${Math.random() * 0.35}s`;
    p.style.animationDuration = `${1.2 + Math.random() * 0.9}s`;
    if (i % 3 === 0) p.style.borderRadius = '50%';
    host.appendChild(p);
  }
  document.body.appendChild(host);
  setTimeout(() => host.remove(), 2600);
}

// ------------------------------------------------------------------ dialog
/** Promise-based confirm dialog using <dialog>; resolves true when confirmed. */
export function confirmDialog({ title, body, confirmLabel = t('common.confirm'), danger = false }) {
  return new Promise((resolve) => {
    const dlg = document.createElement('dialog');
    dlg.className = 'modal';
    dlg.setAttribute('aria-labelledby', 'dlg-title');
    dlg.innerHTML = `
      <h2 id="dlg-title">${esc(title)}</h2>
      <p>${esc(body)}</p>
      <div class="btn-row">
        <button type="button" class="btn btn--light" data-v="0">${esc(t('common.cancel'))}</button>
        <button type="button" class="btn ${danger ? 'btn--danger' : 'btn--primary'}" data-v="1">${esc(confirmLabel)}</button>
      </div>`;
    document.body.appendChild(dlg);
    const done = (v) => {
      dlg.close();
      dlg.remove();
      resolve(v);
    };
    dlg.addEventListener('click', (e) => {
      const b = e.target.closest('button[data-v]');
      if (b) done(b.dataset.v === '1');
      else if (e.target === dlg) done(false);
    });
    dlg.addEventListener('cancel', (e) => {
      e.preventDefault();
      done(false);
    });
    dlg.showModal();
    dlg.querySelector('[data-v="0"]').focus();
  });
}

export function debounce(fn, ms) {
  let id;
  return (...args) => {
    clearTimeout(id);
    id = setTimeout(() => fn(...args), ms);
  };
}

export function shuffle(arr) {
  const a = arr.slice();
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

export async function copyText(text) {
  try {
    await navigator.clipboard.writeText(text);
    return true;
  } catch {
    const ta = document.createElement('textarea');
    ta.value = text;
    ta.style.position = 'fixed';
    ta.style.opacity = '0';
    document.body.appendChild(ta);
    ta.select();
    const ok = document.execCommand('copy');
    ta.remove();
    return ok;
  }
}

/** Translated UI string rendered safely as HTML (escaped, Latin runs bidi-isolated in Persian). */
export function tx(key, vars) {
  return rich(t(key, vars));
}
