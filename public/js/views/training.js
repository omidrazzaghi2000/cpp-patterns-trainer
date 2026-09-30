import { t, L, other, num, lang } from '../i18n.js';
import { api } from '../api.js';
import { store } from '../store.js';
import { esc, icon, rich, tx, shuffle, confetti } from '../util.js';
import { mountQuiz } from '../components/quiz.js';

const MODES = [
  { id: 'quiz', icon: 'list-checks', block: 'block--indigo' },
  { id: 'match', icon: 'target', cat: 'behavioral' },
  { id: 'flash', icon: 'brain', cat: 'structural' },
  { id: 'code', icon: 'puzzle', block: 'block--orange' },
];

// Sessions survive re-renders (language switch) until finished or ended.
const sessions = {};
const setup = { cats: [], count: 10 };

export function render(parentCtx) {
  // Fresh host per render so listeners from a previous screen (setup → session) are dropped.
  const host = document.createElement('div');
  parentCtx.root.replaceChildren(host);
  const ctx = { ...parentCtx, root: host, rerender: () => render(parentCtx) };
  const mode = ctx.params.mode;
  if (!mode) return renderModes(ctx);
  if (mode === 'code') return renderCodeList(ctx);
  if (!['quiz', 'match', 'flash'].includes(mode)) {
    location.replace('#/training');
    return undefined;
  }
  ctx.setTitle(t(`training.modes.${mode}`)[0]);
  if (!sessions[mode]) return renderSetup(ctx, mode);
  return renderSession(ctx, mode);
}

function modeHeader(mode) {
  const [title, sub] = t(`training.modes.${mode}`);
  return `<nav class="crumbs" aria-label="Breadcrumb"><a href="#/training">${esc(t('training.title'))}</a>${icon('chevron-right', 'flip-rtl')}<span aria-current="page">${esc(title)}</span></nav>
    <header class="page-head"><h1>${rich(title)}</h1><p>${rich(sub)}</p></header>`;
}

// ------------------------------------------------------------------ modes
function renderModes(ctx) {
  const { root } = ctx;
  ctx.setTitle(t('training.title'));
  const tr = store.get().training;
  root.innerHTML = `
    <header class="page-head"><h1>${tx('training.title')}</h1><p>${tx('training.sub')}</p></header>
    <div class="mode-grid">
      ${MODES.map((m) => {
        const [title, sub] = t(`training.modes.${m.id}`);
        return `<a class="block mode-card ${m.block || ''}" ${m.cat ? `data-cat="${m.cat}"` : ''} href="#/training/${m.id}">
          <span class="mode-icon">${icon(m.icon)}</span>
          <h3>${rich(title)}</h3>
          <p>${rich(sub)}</p>
          <span class="go">${esc(t('training.play'))}${icon('arrow-right', 'flip-rtl')}</span>
        </a>`;
      }).join('')}
    </div>
    <div class="stats-row">
      <div class="clay stat tone-indigo"><span class="stat-icon">${icon('dumbbell')}</span><span class="stat-value">${num(tr.sessions)}</span><span class="stat-label">${esc(t('training.title'))}</span></div>
      <div class="clay stat tone-green"><span class="stat-icon">${icon('circle-check')}</span><span class="stat-value">${num(tr.answered ? Math.round((tr.correct / tr.answered) * 100) : 0)}${lang() === 'fa' ? '٪' : '%'}</span><span class="stat-label">${esc(t('training.score'))}</span></div>
      <div class="clay stat tone-red"><span class="stat-icon">${icon('flame')}</span><span class="stat-value">${num(tr.bestStreak)}</span><span class="stat-label">${esc(t('training.bestStreak', { n: '' }).trim())}</span></div>
      <div class="clay stat tone-amber"><span class="stat-icon">${icon('star')}</span><span class="stat-value">${num(tr.correct)}</span><span class="stat-label">${esc(t('quiz.correct'))}</span></div>
    </div>`;
}

// ------------------------------------------------------------------ setup
function renderSetup(ctx, mode) {
  const { root, categories } = ctx;
  root.innerHTML = `${modeHeader(mode)}
    <section class="card setup" aria-labelledby="setup-title">
      <h2 id="setup-title">${esc(t('training.setupTitle'))}</h2>
      <div class="setup-row">
        <span class="label" id="cats-label">${esc(t('training.categories'))}</span>
        <div class="filter-chips" role="group" aria-labelledby="cats-label">
          <button type="button" class="chip-toggle" data-all aria-pressed="${setup.cats.length === 0}">${esc(t('common.all'))}</button>
          ${categories.map((c) => `<button type="button" class="chip-toggle" data-cat="${esc(c.id)}" data-c="${esc(c.id)}" aria-pressed="${setup.cats.includes(c.id)}"><span class="dot" aria-hidden="true"></span>${rich(L(c.name))}</button>`).join('')}
        </div>
      </div>
      <div class="setup-row">
        <span class="label" id="count-label">${esc(t('training.count'))}</span>
        <div class="segmented" role="group" aria-labelledby="count-label">
          ${[5, 10, 20].map((n) => `<button type="button" data-n="${n}" aria-pressed="${setup.count === n}">${num(n)}</button>`).join('')}
        </div>
      </div>
      <p class="sr-only" id="setup-error" role="alert"></p>
      <button type="button" class="btn btn--accent btn--lg" id="start">${icon('play')}${esc(t('training.start'))}</button>
    </section>`;

  root.addEventListener('click', async (e) => {
    const c = e.target.closest('[data-c]');
    if (c) {
      const id = c.dataset.c;
      setup.cats = setup.cats.includes(id) ? setup.cats.filter((x) => x !== id) : [...setup.cats, id];
      if (setup.cats.length === categories.length) setup.cats = [];
      root.querySelectorAll('[data-c]').forEach((b) => b.setAttribute('aria-pressed', String(setup.cats.includes(b.dataset.c))));
      root.querySelector('[data-all]').setAttribute('aria-pressed', String(setup.cats.length === 0));
    }
    if (e.target.closest('[data-all]')) {
      setup.cats = [];
      root.querySelectorAll('[data-c]').forEach((b) => b.setAttribute('aria-pressed', 'false'));
      root.querySelector('[data-all]').setAttribute('aria-pressed', 'true');
    }
    const n = e.target.closest('[data-n]');
    if (n) {
      setup.count = Number(n.dataset.n);
      root.querySelectorAll('[data-n]').forEach((b) => b.setAttribute('aria-pressed', String(b === n)));
    }
    const start = e.target.closest('#start');
    if (start) {
      start.disabled = true;
      start.innerHTML = `${icon('loader-circle', 'spin')}${esc(t('loading'))}`;
      try {
        await startSession(ctx, mode);
        ctx.rerender();
      } catch {
        start.disabled = false;
        start.innerHTML = `${icon('play')}${esc(t('training.start'))}`;
        root.querySelector('#setup-error').textContent = t('loadFailed');
        root.querySelector('#setup-error').classList.remove('sr-only');
      }
    }
  });
}

async function startSession(ctx, mode) {
  const params = { count: setup.count };
  if (setup.cats.length) params.categories = setup.cats.join(',');
  if (mode === 'quiz') {
    const questions = await api.quiz(params);
    sessions.quiz = { questions, state: { index: 0, picks: [] }, streak: 0, best: 0 };
  } else if (mode === 'match') {
    const rounds = await api.match(params);
    const questions = rounds.map((r) => ({
      question: r.scenario,
      options: { en: r.options.map((o) => o.name.en), fa: r.options.map((o) => o.name.fa) },
      answer: r.answer,
      explanation: {
        en: `**${r.pattern.name.en}** — ${r.pattern.intent.en}`,
        fa: `**${r.pattern.name.fa}** — ${r.pattern.intent.fa}`,
      },
    }));
    sessions.match = { questions, state: { index: 0, picks: [] }, streak: 0, best: 0 };
  } else if (mode === 'flash') {
    const pool = ctx.patterns.filter((p) => !setup.cats.length || setup.cats.includes(p.category));
    sessions.flash = { cards: shuffle(pool).slice(0, setup.count), index: 0, flipped: false, known: [], learning: [], reported: false };
  }
}

// ------------------------------------------------------------------ session
function renderSession(ctx, mode) {
  const { root } = ctx;
  const s = sessions[mode];

  if (mode === 'flash') return renderFlash(ctx, s);

  if (!s.questions.length) {
    root.innerHTML = `${modeHeader(mode)}<div class="clay empty-state" style="padding:32px"><p>${esc(t('training.emptyPool'))}</p><a class="btn btn--primary" href="#/training">${esc(t('training.backToModes'))}</a></div>`;
    delete sessions[mode];
    return undefined;
  }

  root.innerHTML = `${modeHeader(mode)}
    <div class="session-bar">
      <button type="button" class="btn btn--light btn--sm" id="quit">${icon('x')}${esc(t('training.quit'))}</button>
      <span style="flex:1"></span>
      <span class="streak" id="streak" aria-label="${esc(t('training.streak'))}">${icon('flame')}<span>${num(s.streak)}</span></span>
    </div>
    ${mode === 'match' ? `<h2 style="font-size:1.1rem;display:flex;gap:8px;align-items:center;justify-content:center">${icon('target')}${esc(t('training.pickPattern'))}</h2>` : ''}
    <div id="quiz-root"></div>`;

  const streakEl = root.querySelector('#streak');
  mountQuiz(root.querySelector('#quiz-root'), {
    questions: s.questions,
    state: s.state,
    showPattern: mode === 'quiz',
    onAnswer: (ok) => {
      s.streak = ok ? s.streak + 1 : 0;
      s.best = Math.max(s.best, s.streak);
      streakEl.querySelector('span').textContent = num(s.streak);
      streakEl.classList.remove('bump');
      if (ok) {
        void streakEl.offsetWidth;
        streakEl.classList.add('bump');
      }
    },
    onComplete: (score, total) => {
      store.recordTraining({ mode, correct: score, total, bestStreak: s.best });
    },
    onRestart: async () => {
      await startSession(ctx, mode);
      ctx.rerender();
    },
    extraResultButtons: `<a class="btn btn--light" href="#/training">${icon('arrow-left', 'flip-rtl')}${esc(t('training.backToModes'))}</a>`,
  });

  root.querySelector('#quit').addEventListener('click', () => {
    delete sessions[mode];
    location.hash = '#/training';
  });
  return undefined;
}

// ------------------------------------------------------------------ flashcards
function renderFlash(ctx, s) {
  const { root } = ctx;

  if (s.index >= s.cards.length) {
    if (!s.reported) {
      s.reported = true;
      store.recordTraining({ mode: 'flash', correct: s.known.length, total: s.cards.length, bestStreak: 0 });
      if (s.known.length === s.cards.length) confetti();
    }
    const pct = s.cards.length ? Math.round((s.known.length / s.cards.length) * 100) : 0;
    const learning = s.cards.filter((c) => s.learning.includes(c.id));
    root.innerHTML = `${modeHeader('flash')}
      <div class="stack" style="max-width:820px;margin-inline:auto">
        <section class="block result-block" data-cat="structural">
          <div class="score-ring" style="--p:${pct}"><strong>${num(s.known.length)}/${num(s.cards.length)}</strong></div>
          <h2 tabindex="-1">${esc(t('training.flashDone'))}</h2>
          <p>${esc(t('training.flashScore', { n: s.known.length, m: s.cards.length }))}</p>
          <div class="btn-row" style="justify-content:center">
            <button type="button" class="btn btn--light" id="again">${icon('rotate-ccw')}${esc(t('training.playAgain'))}</button>
            <a class="btn btn--light" href="#/training">${icon('arrow-left', 'flip-rtl')}${esc(t('training.backToModes'))}</a>
          </div>
        </section>
        ${learning.length ? `<section class="card"><h3>${icon('book-open')}${esc(t('training.again'))}</h3>
          <div class="challenge-list">${learning.map((p) => `<a class="challenge-row" data-cat="${esc(p.category)}" href="#/p/${esc(p.id)}"><span class="icon-tile" aria-hidden="true">${icon(p.icon)}</span><span><strong>${rich(L(p.name))}</strong><small>${rich(L(p.tagline))}</small></span>${icon('arrow-right', 'flip-rtl')}</a>`).join('')}</div>
        </section>` : ''}
      </div>`;
    root.querySelector('h2').focus();
    root.querySelector('#again').addEventListener('click', async () => {
      await startSession(ctx, 'flash');
      ctx.rerender();
    });
    return undefined;
  }

  const card = s.cards[s.index];
  const pct = Math.round((s.index / s.cards.length) * 100);
  root.innerHTML = `${modeHeader('flash')}
    <div class="flash-wrap">
      <div class="session-bar">
        <button type="button" class="btn btn--light btn--sm" id="quit">${icon('x')}${esc(t('training.quit'))}</button>
        <div class="meter meter--light" role="progressbar" aria-valuemin="0" aria-valuemax="${s.cards.length}" aria-valuenow="${s.index}" aria-label="${esc(t('quiz.progress', { n: s.index + 1, m: s.cards.length }))}"><span style="--value:${pct}%"></span></div>
        <span class="quiz-count">${num(s.index + 1)}/${num(s.cards.length)}</span>
      </div>
      <button type="button" class="flashcard ${s.flipped ? 'flipped' : ''}" id="card" aria-label="${esc(t('training.flip'))}" aria-describedby="flash-live">
        <span class="flash-face front clay">
          <span class="chip chip--cat" data-cat="${esc(card.category)}">${icon('circle-help')}${esc(t('training.whichPattern'))}</span>
          <span class="flash-q">${rich(L(card.intent))}</span>
          <span class="flash-hint">${icon('repeat-2')}${tx('training.tapToFlip')}</span>
        </span>
        <span class="flash-face back block" data-cat="${esc(card.category)}">
          <span class="phero-icon" aria-hidden="true">${icon(card.icon)}</span>
          <strong class="flash-name">${rich(L(card.name))}</strong>
          <span lang="${lang() === 'fa' ? 'en' : 'fa'}" style="font-weight:700;opacity:.9">${esc(other(card.name))}</span>
          <span style="font-size:1.1rem">${rich(L(card.tagline))}</span>
        </span>
      </button>
      <p class="sr-only" id="flash-live" aria-live="polite">${s.flipped ? esc(`${L(card.name)} — ${L(card.tagline)}`) : ''}</p>
      <div class="flash-actions">
        ${s.flipped
          ? `<button type="button" class="btn btn--light btn--lg" data-verdict="again">${icon('rotate-ccw')}${esc(t('training.again'))}</button>
             <button type="button" class="btn btn--success btn--lg" data-verdict="knew">${icon('check')}${esc(t('training.knew'))}</button>`
          : `<button type="button" class="btn btn--primary btn--lg" id="flip">${icon('repeat-2')}${esc(t('training.flip'))}</button>`}
      </div>
    </div>`;

  const flip = () => {
    s.flipped = !s.flipped;
    renderFlash(ctx, s);
    const next = root.querySelector('[data-verdict="knew"]') || root.querySelector('#flip');
    if (next) next.focus();
  };
  root.querySelector('#card').addEventListener('click', flip);
  const flipBtn = root.querySelector('#flip');
  if (flipBtn) flipBtn.addEventListener('click', flip);
  root.querySelectorAll('[data-verdict]').forEach((b) => b.addEventListener('click', () => {
    (b.dataset.verdict === 'knew' ? s.known : s.learning).push(card.id);
    s.index += 1;
    s.flipped = false;
    renderFlash(ctx, s);
    const f = root.querySelector('#flip') || root.querySelector('h2');
    if (f) f.focus();
  }));
  root.querySelector('#quit').addEventListener('click', () => {
    delete sessions.flash;
    location.hash = '#/training';
  });
  return undefined;
}

// ------------------------------------------------------------------ code list
function renderCodeList(ctx) {
  const { root, patterns, categories } = ctx;
  ctx.setTitle(t('training.codeTitle'));
  const solved = patterns.filter((p) => store.isSolved(p.id)).length;
  const pct = patterns.length ? Math.round((solved / patterns.length) * 100) : 0;
  root.innerHTML = `${modeHeader('code')}
    <section class="card" style="margin-bottom:32px">
      <div class="cat-progress-row" style="grid-template-columns:1fr">
        <div>
          <div class="row-top"><span>${esc(t('training.codeTitle'))}</span><span>${esc(t('training.solvedCount', { n: solved, m: patterns.length }))}</span></div>
          <div class="meter meter--light" role="progressbar" aria-valuemin="0" aria-valuemax="${patterns.length}" aria-valuenow="${solved}" aria-label="${esc(t('training.solvedCount', { n: solved, m: patterns.length }))}"><span style="--value:${pct}%"></span></div>
        </div>
      </div>
    </section>
    ${categories.map((c) => {
      const list = patterns.filter((p) => p.category === c.id);
      if (!list.length) return '';
      return `<section class="cat-section" data-cat="${esc(c.id)}">
        <h2 class="cat-heading"><span class="cat-badge" aria-hidden="true">${icon(c.icon)}</span>${rich(L(c.title))}</h2>
        <div class="challenge-list">${list.map((p) => `<a class="challenge-row" data-cat="${esc(p.category)}" href="#/p/${esc(p.id)}/challenge">
          <span class="icon-tile" aria-hidden="true">${icon(p.icon)}</span>
          <span><strong>${rich(L(p.exerciseTitle || p.name))}</strong><small>${rich(L(p.name))} · ${esc(t(`diff.${p.difficulty}`))}</small></span>
          ${store.isSolved(p.id) ? `<span class="chip chip--success">${icon('circle-check')}${esc(t('catalog.solved'))}</span>` : icon('arrow-right', 'flip-rtl')}
        </a>`).join('')}</div>
      </section>`;
    }).join('')}`;
  return undefined;
}
