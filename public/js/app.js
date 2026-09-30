import { initLang, setLang, lang, t, num } from './i18n.js';
import { api } from './api.js';
import { store, levelInfo } from './store.js';
import { esc, icon, rich, toast, confetti } from './util.js';
import * as home from './views/home.js';
import * as catalog from './views/catalog.js';
import * as pattern from './views/pattern.js';
import * as training from './views/training.js';
import * as progress from './views/progress.js';
import * as playground from './views/playground.js';

const ROUTES = [
  { re: /^\/?$/, view: home, nav: 'home' },
  { re: /^\/patterns$/, view: catalog, nav: 'patterns' },
  { re: /^\/p\/([a-z0-9-]+)(?:\/([a-z]+))?$/, view: pattern, nav: 'patterns', keys: ['id', 'tab'] },
  { re: /^\/training(?:\/([a-z]+))?$/, view: training, nav: 'training', keys: ['mode'] },
  { re: /^\/progress$/, view: progress, nav: 'progress' },
  { re: /^\/playground$/, view: playground, nav: 'playground' },
];

const NAV = [
  { id: 'home', href: '#/', icon: 'house' },
  { id: 'patterns', href: '#/patterns', icon: 'layout-grid' },
  { id: 'training', href: '#/training', icon: 'dumbbell' },
  { id: 'playground', href: '#/playground', icon: 'terminal' },
  { id: 'progress', href: '#/progress', icon: 'trophy' },
];

const app = {
  meta: null,
  patterns: [],
  byId: new Map(),
  categories: [],
  catById: new Map(),
  cleanup: null,
  currentNav: 'home',
  lastPath: null,
};

// ------------------------------------------------------------------ shell
function renderShell() {
  document.getElementById('skip-link').textContent = t('skip');
  const lvl = levelInfo();
  document.getElementById('topbar').innerHTML = `
    <div class="container topbar-inner">
      <a class="brand" href="#/" aria-label="${esc(t('brand'))} — ${esc(t('nav.home'))}">
        <span class="brand-mark" aria-hidden="true">${icon('code-xml')}</span>
        <span class="brand-text" dir="ltr"><b>C++</b> Patterns</span>
      </a>
      <nav class="mainnav" aria-label="${esc(t('nav.main'))}">
        ${NAV.map((n) => `<a class="navlink" href="${n.href}" data-nav="${n.id}">${icon(n.icon)}<span>${esc(t(`nav.${n.id}`))}</span></a>`).join('')}
      </nav>
      <div class="topbar-actions">
        <a class="xp-pill" href="#/progress" id="xp-pill" aria-label="${esc(t('level'))} ${num(lvl.level)}, ${num(lvl.xp)} ${esc(t('xp'))}">
          <span class="lvl">${num(lvl.level)}</span>
          <span class="xp-num">${num(lvl.xp)} ${esc(t('xp'))}</span>
          ${icon('star')}
        </a>
        <div class="lang-toggle" role="group" aria-label="${esc(t('language'))}">
          <button type="button" lang="en" data-lang="en" aria-pressed="${lang() === 'en'}">EN</button>
          <button type="button" lang="fa" data-lang="fa" aria-pressed="${lang() === 'fa'}">فا</button>
        </div>
      </div>
    </div>`;

  const bottom = document.getElementById('bottom-nav');
  bottom.setAttribute('aria-label', t('nav.main'));
  bottom.innerHTML = NAV.map((n) => `<a href="${n.href}" data-nav="${n.id}">${icon(n.icon)}<span>${esc(t(`nav.${n.id}`))}</span></a>`).join('');

  const c = app.meta && app.meta.compiler;
  document.getElementById('footer').innerHTML = `<div class="container">
    <span>${rich(t('footer.made'))}</span>
    <span class="chip">${icon(c && c.available ? 'cpu' : 'triangle-alert')}<span dir="ltr">${c && c.available ? esc(`${c.version} · -std=${c.standard}`) : esc(t('footer.noCompiler'))}</span></span>
  </div>`;
  markNav();
}

function markNav() {
  document.querySelectorAll('[data-nav]').forEach((a) => {
    if (a.dataset.nav === app.currentNav) a.setAttribute('aria-current', 'page');
    else a.removeAttribute('aria-current');
  });
}

function updateXpPill() {
  const pill = document.getElementById('xp-pill');
  if (!pill) return;
  const lvl = levelInfo();
  pill.querySelector('.lvl').textContent = num(lvl.level);
  pill.querySelector('.xp-num').textContent = `${num(lvl.xp)} ${t('xp')}`;
  pill.setAttribute('aria-label', `${t('level')} ${num(lvl.level)}, ${num(lvl.xp)} ${t('xp')}`);
}

// ------------------------------------------------------------------ router
function parseHash() {
  const raw = decodeURIComponent(location.hash.replace(/^#/, '')) || '/';
  const [path, qs] = raw.split('?');
  return { path: path || '/', query: new URLSearchParams(qs || '') };
}

function route({ keepScroll = false } = {}) {
  const { path, query } = parseHash();
  let match = null;
  let params = {};
  for (const r of ROUTES) {
    const m = path.match(r.re);
    if (m) {
      match = r;
      (r.keys || []).forEach((k, i) => { params[k] = m[i + 1]; });
      break;
    }
  }
  if (!match) {
    location.replace('#/');
    return;
  }

  if (typeof app.cleanup === 'function') app.cleanup();
  app.cleanup = null;
  app.currentNav = match.nav;
  markNav();

  const main = document.getElementById('main');
  main.innerHTML = '<div class="container" id="view"></div>';
  const root = document.getElementById('view');
  const scrollY = window.scrollY;
  const navigated = app.lastPath !== null && app.lastPath !== path;
  app.lastPath = path;

  const ctx = { ...app, root, params, query, setTitle };
  const result = match.view.render(ctx);
  app.cleanup = typeof result === 'function' ? result : null;

  if (keepScroll) window.scrollTo(0, scrollY);
  else if (navigated) {
    window.scrollTo(0, 0);
    main.focus({ preventScroll: true });
  }
}

function setTitle(title) {
  document.title = title ? `${title} · C++ Patterns Trainer` : 'C++ Patterns Trainer';
}

// ------------------------------------------------------------------ events
document.addEventListener('click', (e) => {
  const btn = e.target.closest('[data-lang]');
  if (btn && btn.dataset.lang !== lang()) {
    setLang(btn.dataset.lang);
    renderShell();
    route({ keepScroll: true });
    const again = document.querySelector(`[data-lang="${btn.dataset.lang}"]`);
    if (again) again.focus();
  }
});

store.subscribe((state, change) => {
  updateXpPill();
  if (!change) return;
  if (change.xp > 0) toast(t('toast.xp', { n: change.xp }), { iconName: 'star', tone: 'amber', timeout: 2200 });
  for (const b of change.badges) {
    const [name] = t(`badges.${b.id}`);
    toast(t('toast.badge', { name }), { iconName: b.icon, tone: b.tone, timeout: 4200 });
  }
  if (change.levelUp) {
    toast(t('toast.levelUp', { n: change.levelUp }), { iconName: 'trophy', tone: 'amber', timeout: 4200 });
    confetti();
  }
});

window.addEventListener('hashchange', () => route());

// ------------------------------------------------------------------ boot
async function boot() {
  initLang();
  renderShell();
  try {
    const [meta, patterns] = await Promise.all([api.meta(), api.patterns()]);
    app.meta = meta;
    app.patterns = patterns;
    app.byId = new Map(patterns.map((p) => [p.id, p]));
    app.categories = meta.categories;
    app.catById = new Map(meta.categories.map((c) => [c.id, c]));
    store.init(patterns);
    renderShell();
    route();
  } catch (err) {
    console.error(err);
    document.getElementById('main').innerHTML = `<div class="container empty-state" role="alert">
      <span class="brand-mark" aria-hidden="true">${icon('triangle-alert')}</span>
      <p>${esc(t('loadFailed'))}</p>
      <button type="button" class="btn btn--primary" id="retry">${icon('rotate-ccw')}${esc(t('retry'))}</button>
    </div>`;
    document.getElementById('retry').addEventListener('click', () => location.reload());
  }
}

boot();
