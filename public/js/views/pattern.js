import { t, L, other, num, lang } from '../i18n.js';
import { api, isStatic } from '../api.js';
import { store } from '../store.js';
import { esc, icon, rich, tx, difficultyDots, toast, confetti, confirmDialog, copyText } from '../util.js';
import { renderDiagram, renderLegend } from '../components/diagram.js';
import { mountQuiz } from '../components/quiz.js';
import { createEditor, codeToolbar, consoleShell, statusChip, renderRunResult, renderRunError, renderRunning } from '../components/editor.js';

const TABS = [
  { id: 'overview', icon: 'book-open' },
  { id: 'structure', icon: 'network' },
  { id: 'code', icon: 'code-xml' },
  { id: 'quiz', icon: 'list-checks' },
  { id: 'challenge', icon: 'puzzle' },
];

// Per-pattern UI state that should survive re-renders (e.g. language switch).
const quizStates = new Map();
const hintsShown = new Map();
const lastRuns = new Map();

export function render(ctx) {
  const { root, params, catById } = ctx;
  const id = params.id;
  let disposed = false;
  let panelCleanup = null;

  root.innerHTML = `<div class="skeleton" style="height:260px"></div><div class="skeleton" style="height:64px;margin-top:24px;border-radius:999px"></div><div class="skeleton" style="height:320px;margin-top:24px"></div>`;

  api.pattern(id).then((p) => {
    if (disposed) return;
    const cat = catById.get(p.category);
    ctx.setTitle(L(p.name));
    const tab = TABS.some((x) => x.id === params.tab) ? params.tab : 'overview';

    root.innerHTML = `
      <nav class="crumbs" aria-label="Breadcrumb">
        <a href="#/patterns">${esc(t('nav.patterns'))}</a>${icon('chevron-right', 'flip-rtl')}
        <a href="#/patterns?cat=${esc(p.category)}">${rich(L(cat.name))}</a>${icon('chevron-right', 'flip-rtl')}
        <span aria-current="page">${rich(L(p.name))}</span>
      </nav>
      <section class="block phero" data-cat="${esc(p.category)}" aria-labelledby="p-title">
        <span class="phero-icon" aria-hidden="true">${icon(p.icon)}</span>
        <div>
          <div class="phero-meta">
            <span class="chip chip--light">${icon(cat.icon)}${rich(L(cat.name))}</span>
            <span class="chip chip--light">${icon('gauge')}${esc(t(`diff.${p.difficulty}`))}</span>
          </div>
          <h1 id="p-title">${rich(L(p.name))}</h1>
          <p class="alt-name" lang="${lang() === 'fa' ? 'en' : 'fa'}">${esc(other(p.name))}</p>
          <p class="tagline">${rich(L(p.tagline))}</p>
          <div class="journey" id="journey"></div>
        </div>
        <div class="phero-actions">
          <button type="button" class="btn btn--light" id="learned-btn"></button>
        </div>
      </section>
      <div class="tabs-wrap">
        <div class="tabs" role="tablist" aria-label="${esc(t('pattern.tabsLabel'))}" data-cat="${esc(p.category)}">
          ${TABS.map((x) => `<button type="button" role="tab" class="tab" id="tab-${x.id}" data-tab="${x.id}" aria-controls="panel" aria-selected="${x.id === tab}" tabindex="${x.id === tab ? 0 : -1}">${icon(x.icon)}<span>${esc(t(`pattern.tabs.${x.id}`))}</span></button>`).join('')}
        </div>
      </div>
      <div id="panel" class="tabpanel" role="tabpanel" tabindex="-1" data-cat="${esc(p.category)}"></div>
      <nav class="pager" aria-label="${esc(t('pattern.prev'))} / ${esc(t('pattern.next'))}">
        ${p.prev ? `<a href="#/p/${esc(p.prev.id)}" data-cat="${esc(p.prev.category)}">${icon('arrow-left', 'flip-rtl')}<span class="icon-tile" aria-hidden="true">${icon(p.prev.icon)}</span><span><small>${esc(t('pattern.prev'))}</small><strong>${rich(L(p.prev.name))}</strong></span></a>` : '<span></span>'}
        ${p.next ? `<a class="next" href="#/p/${esc(p.next.id)}" data-cat="${esc(p.next.category)}"><span><small>${esc(t('pattern.next'))}</small><strong>${rich(L(p.next.name))}</strong></span><span class="icon-tile" aria-hidden="true">${icon(p.next.icon)}</span>${icon('arrow-right', 'flip-rtl')}</a>` : ''}
      </nav>`;

    const panel = root.querySelector('#panel');
    const learnedBtn = root.querySelector('#learned-btn');

    function updateHeroState() {
      const learned = store.isLearned(p.id);
      learnedBtn.setAttribute('aria-pressed', String(learned));
      learnedBtn.innerHTML = `${icon(learned ? 'bookmark-check' : 'bookmark')}${esc(t(learned ? 'pattern.learned' : 'pattern.markLearned'))}`;
      const q = store.quizBest(p.id);
      const solved = store.isSolved(p.id);
      root.querySelector('#journey').innerHTML = [
        { done: learned, icon: 'book-open', label: t('pattern.journey.read') },
        { done: q && q.best === q.total, icon: 'list-checks', label: q ? `${t('pattern.journey.quiz')} ${num(q.best)}/${num(q.total)}` : t('pattern.journey.quiz') },
        { done: solved, icon: 'puzzle', label: t('pattern.journey.challenge') },
      ].map((s) => `<span class="journey-step ${s.done ? 'done' : ''}">${icon(s.done ? 'circle-check' : s.icon)}<span>${esc(s.label)}</span></span>`).join('');
    }
    updateHeroState();

    learnedBtn.addEventListener('click', () => {
      const wasLearned = store.isLearned(p.id);
      store.toggleLearned(p.id);
      updateHeroState();
      if (!wasLearned) toast(t('toast.learned'), { iconName: 'bookmark-check', tone: 'green', timeout: 2000 });
    });

    function selectTab(next, { focusPanel = false, push = true } = {}) {
      if (typeof panelCleanup === 'function') panelCleanup();
      panelCleanup = null;
      root.querySelectorAll('[role="tab"]').forEach((b) => {
        const on = b.dataset.tab === next;
        b.setAttribute('aria-selected', String(on));
        b.tabIndex = on ? 0 : -1;
      });
      panel.setAttribute('aria-labelledby', `tab-${next}`);
      if (push) history.replaceState(null, '', `#/p/${p.id}${next === 'overview' ? '' : `/${next}`}`);
      panel.style.animation = 'none';
      void panel.offsetWidth; // restart the entrance animation
      panel.style.animation = '';
      // A fresh host per tab, so listeners from the previous tab are dropped with it.
      const host = document.createElement('div');
      panel.replaceChildren(host);
      panelCleanup = PANELS[next](host, p, { updateHeroState, selectTab });
      if (focusPanel) panel.focus({ preventScroll: true });
    }

    const tablist = root.querySelector('[role="tablist"]');
    tablist.addEventListener('click', (e) => {
      const b = e.target.closest('[role="tab"]');
      if (!b) return;
      selectTab(b.dataset.tab);
      const top = root.querySelector('.tabs-wrap').getBoundingClientRect().top + window.scrollY - 90;
      if (window.scrollY > top) window.scrollTo({ top });
    });
    tablist.addEventListener('keydown', (e) => {
      const tabs = [...tablist.querySelectorAll('[role="tab"]')];
      const i = tabs.indexOf(document.activeElement);
      if (i < 0) return;
      const rtl = document.documentElement.dir === 'rtl';
      let j = null;
      if (e.key === (rtl ? 'ArrowLeft' : 'ArrowRight')) j = (i + 1) % tabs.length;
      else if (e.key === (rtl ? 'ArrowRight' : 'ArrowLeft')) j = (i - 1 + tabs.length) % tabs.length;
      else if (e.key === 'Home') j = 0;
      else if (e.key === 'End') j = tabs.length - 1;
      if (j === null) return;
      e.preventDefault();
      tabs[j].focus();
      selectTab(tabs[j].dataset.tab);
    });

    selectTab(tab, { push: false });
  }).catch((err) => {
    if (disposed) return;
    ctx.setTitle(t('pattern.notFound'));
    root.innerHTML = `<div class="clay empty-state" style="padding:32px">${icon('circle-help', 'icon--xl')}<h1>${esc(err.status === 404 ? t('pattern.notFound') : t('loadFailed'))}</h1><a class="btn btn--primary" href="#/patterns">${icon('arrow-left', 'flip-rtl')}${esc(t('pattern.backToLibrary'))}</a></div>`;
  });

  return () => {
    disposed = true;
    if (typeof panelCleanup === 'function') panelCleanup();
  };
}

// ------------------------------------------------------------------ panels

function listCard(title, iconName, tone, items, itemIcon) {
  return `<article class="card tone-${tone}">
    <h3><span class="card-title-icon">${icon(iconName)}</span>${esc(title)}</h3>
    <ul class="list-clean">${items.map((s) => `<li>${icon(itemIcon)}<span>${rich(s)}</span></li>`).join('')}</ul>
  </article>`;
}

const PANELS = {
  overview(panel, p) {
    panel.innerHTML = `<div class="stack">
      <section class="block intent-card" data-cat="${esc(p.category)}">
        <span class="label">${icon('target')}${esc(t('pattern.intent'))}</span>
        <p class="intent-text">${rich(L(p.intent))}</p>
        ${p.aka ? `<p style="margin:16px 0 0;opacity:.95"><strong>${esc(t('pattern.aka'))}:</strong> ${rich(L(p.aka))}</p>` : ''}
      </section>
      <div class="grid-2">
        <article class="card tone-red"><h3><span class="card-title-icon">${icon('triangle-alert')}</span>${esc(t('pattern.problem'))}</h3><div class="prose"><p>${rich(L(p.problem))}</p></div></article>
        <article class="card tone-green"><h3><span class="card-title-icon">${icon('lightbulb')}</span>${esc(t('pattern.solution'))}</h3><div class="prose"><p>${rich(L(p.solution))}</p></div></article>
      </div>
      <article class="card tone-pink"><h3><span class="card-title-icon">${icon('heart')}</span>${esc(t('pattern.analogy'))}</h3><div class="prose"><p>${rich(L(p.analogy))}</p></div></article>
      <div class="grid-3">
        ${listCard(t('pattern.whenToUse'), 'circle-help', 'indigo', L(p.whenToUse), 'check')}
        ${listCard(t('pattern.pros'), 'circle-check', 'green', L(p.pros), 'check')}
        ${listCard(t('pattern.cons'), 'circle-x', 'red', L(p.cons), 'x')}
      </div>
      <article class="card tone-violet">
        <h3><span class="card-title-icon">${icon('cpu')}</span>${esc(t('pattern.cppTips'))}</h3>
        <ol class="tips-list">${L(p.cppTips).map((s) => `<li>${rich(s)}</li>`).join('')}</ol>
      </article>
      <article class="card tone-amber">
        <h3><span class="card-title-icon">${icon('map')}</span>${esc(t('pattern.realWorld'))}</h3>
        <ul class="list-clean">${L(p.realWorld).map((s) => `<li>${icon('arrow-up-right', 'flip-rtl')}<span>${rich(s)}</span></li>`).join('')}</ul>
      </article>
      ${p.related && p.related.length ? `<section>
        <h2 style="display:flex;gap:10px;align-items:center">${icon('git-compare')}${esc(t('pattern.related'))}</h2>
        <div class="related-grid">${p.related.filter((r) => r.pattern).map((r) => `<a class="related-card" data-cat="${esc(r.pattern.category)}" href="#/p/${esc(r.id)}">
          <span class="icon-tile" aria-hidden="true">${icon(r.pattern.icon)}</span>
          <span><h4>${rich(L(r.pattern.name))}</h4><p>${rich(L(r.note))}</p></span>
        </a>`).join('')}</div>
      </section>` : ''}
    </div>`;
  },

  structure(panel, p) {
    const legend = t('pattern.legend');
    panel.innerHTML = `<div class="stack">
      <section class="card diagram-card">
        <h2 style="display:flex;gap:10px;align-items:center">${icon('network')}${esc(t('pattern.diagram'))}</h2>
        <div class="diagram-scroll">${renderDiagram(p.diagram, { category: p.category, title: t('pattern.diagramLabel', { name: L(p.name) }) })}</div>
        <div class="diagram-legend" dir="ltr">${renderLegend(p.diagram, legend)}</div>
      </section>
      <section class="card">
        <h2 style="display:flex;gap:10px;align-items:center">${icon('user')}${esc(t('pattern.participants'))}</h2>
        <div class="participants">${p.participants.map((pt) => `<div class="participant"><code dir="ltr">${esc(pt.name)}</code><p>${rich(L(pt.role))}</p></div>`).join('')}</div>
      </section>
      <section class="card">
        <h2 style="display:flex;gap:10px;align-items:center">${icon('list-ordered')}${esc(t('pattern.steps'))}</h2>
        <ol class="steps-list">${L(p.steps).map((s) => `<li><p>${rich(s)}</p></li>`).join('')}</ol>
      </section>
    </div>`;
  },

  code(panel, p) {
    const ex = p.example;
    panel.innerHTML = `<div class="stack">
      <header>
        <h2>${rich(L(ex.title))}</h2>
        <p class="prose" style="color:var(--color-muted-foreground)">${rich(L(ex.description))}</p>
      </header>
      <div class="code-layout">
        <div>
          <div class="code-panel">
            ${codeToolbar({
              file: 'example.cpp',
              buttons: `<button type="button" class="btn btn--ghost btn--sm" data-act="copy">${icon('copy')}<span>${esc(t('code.copy'))}</span></button>
                <button type="button" class="btn btn--accent btn--sm" data-act="run">${icon('play')}<span>${esc(t('code.run'))}</span></button>`,
            })}
            <div class="cm-host" id="ex-editor"></div>
          </div>
          ${consoleShell({ id: 'ex-console', title: t('code.output') })}
        </div>
        <aside class="card walkthrough" aria-labelledby="walk-title">
          <h3 id="walk-title"><span class="card-title-icon">${icon('footprints')}</span>${esc(t('pattern.walkthrough'))}</h3>
          <p style="color:var(--color-muted-foreground);font-size:.92rem">${esc(t('pattern.walkHint'))}</p>
          <ol class="walk-list">${ex.highlights.map((h, i) => `<li><button type="button" class="walk-btn" data-hl="${i}" aria-pressed="false">
            <span class="lines">${esc(`L${h.lines[0]}–${h.lines[1]}`)}</span><span>${rich(L(h.note))}</span>
          </button></li>`).join('')}</ol>
        </aside>
      </div>
    </div>`;

    const editor = createEditor(panel.querySelector('#ex-editor'), { value: ex.code, readOnly: true });
    const con = panel.querySelector('#ex-console');

    const showExpected = () => {
      con.querySelector('.console-status-wrap').innerHTML = statusChip('info', t('code.expected'), 'eye');
      con.querySelector('.console-content').innerHTML = `<pre class="console-body">${esc(ex.output)}</pre><pre class="console-body" style="min-height:0;padding-top:0"><span class="muted">${esc(t('code.notRunYet'))}</span></pre>`;
    };
    const cached = lastRuns.get(`${p.id}:example`);
    if (cached) renderRunResult(con, cached, { expected: ex.output });
    else showExpected();

    const runBtn = panel.querySelector('[data-act="run"]');
    async function run() {
      runBtn.disabled = true;
      renderRunning(con);
      try {
        const result = await api.run(ex.code);
        lastRuns.set(`${p.id}:example`, result);
        renderRunResult(con, result, { expected: ex.output });
        store.recordRun();
      } catch (err) {
        renderRunError(con, err);
      } finally {
        runBtn.disabled = false;
      }
    }

    panel.addEventListener('click', async (e) => {
      const act = e.target.closest('[data-act]');
      if (act && act.dataset.act === 'run') run();
      if (act && act.dataset.act === 'copy') {
        if (await copyText(ex.code)) {
          const label = act.querySelector('span');
          label.textContent = t('code.copied');
          setTimeout(() => { label.textContent = t('code.copy'); }, 1600);
        }
      }
      const hl = e.target.closest('[data-hl]');
      if (hl) {
        const on = hl.getAttribute('aria-pressed') !== 'true';
        panel.querySelectorAll('[data-hl]').forEach((b) => b.setAttribute('aria-pressed', 'false'));
        hl.setAttribute('aria-pressed', String(on));
        editor.highlight(on ? ex.highlights[Number(hl.dataset.hl)].lines : null);
      }
    });
    requestAnimationFrame(() => editor.refresh());
  },

  quiz(panel, p, { updateHeroState }) {
    if (!quizStates.has(p.id)) quizStates.set(p.id, { index: 0, picks: [] });
    const best = store.quizBest(p.id);
    panel.innerHTML = `<p class="prose" style="text-align:center;margin-inline:auto;color:var(--color-muted-foreground)">${esc(t('pattern.quizIntro'))}${best ? ` <strong>${esc(t('pattern.bestScore', { n: best.best, m: best.total }))}</strong>` : ''}</p><div id="quiz-root"></div>`;
    mountQuiz(panel.querySelector('#quiz-root'), {
      questions: p.quiz,
      state: quizStates.get(p.id),
      category: p.category,
      onComplete: (score, total) => {
        store.recordQuiz(p.id, score, total);
        updateHeroState();
      },
      extraResultButtons: `<a class="btn btn--accent" href="#/p/${esc(p.id)}/challenge">${icon('puzzle')}${esc(t('pattern.tabs.challenge'))}</a>`,
    });
  },

  challenge(panel, p, { updateHeroState }) {
    const x = p.exercise;
    const draftKey = `ex.${p.id}`;
    const hints = L(x.hints);
    if (!hintsShown.has(p.id)) hintsShown.set(p.id, 0);

    panel.innerHTML = `<div class="challenge-grid">
      <div class="challenge-side stack">
        <section class="card">
          <span class="chip chip--cat" style="margin-bottom:12px">${icon('puzzle')}${esc(t('challenge.task'))}</span>${store.isSolved(p.id) ? ` <span class="chip chip--success" style="margin-bottom:12px">${icon('circle-check')}${esc(t('challenge.alreadySolved'))}</span>` : ''}
          <h2>${rich(L(x.title))}</h2>
          <p>${rich(L(x.task))}</p>
          <h3 style="font-size:1rem;margin-top:20px;display:flex;gap:8px;align-items:center">${icon('eye')}${esc(t('challenge.expected'))}</h3>
          <pre class="expected-out">${esc(x.expectedOutput)}</pre>
        </section>
        <section class="card">
          <h3><span class="card-title-icon tone-amber" style="--tone-soft:var(--cat-idioms-soft);--tone-text:var(--cat-idioms-text)">${icon('lightbulb')}</span>${esc(t('challenge.hints'))}</h3>
          <ol class="hint-list" id="hints"></ol>
          <button type="button" class="btn btn--light btn--sm" id="hint-btn" style="margin-top:16px"></button>
        </section>
      </div>
      <div>
        <div class="code-panel editor">
          ${codeToolbar({
            file: 'exercise.cpp',
            buttons: `<button type="button" class="btn btn--ghost btn--sm" data-act="reset">${icon('rotate-ccw')}<span>${esc(t('code.reset'))}</span></button>
              <button type="button" class="btn btn--ghost btn--sm" data-act="solution">${icon('eye')}<span>${esc(t('challenge.solution'))}</span></button>
              <button type="button" class="btn btn--primary btn--sm" data-act="run">${icon('play')}<span>${esc(t('code.run'))}</span></button>
              <button type="button" class="btn btn--success btn--sm" data-act="check">${icon('check-check')}<span>${esc(t('challenge.check'))}</span></button>`,
          })}
          <div class="cm-host" id="x-editor"></div>
          <div class="editor-hint">${icon('keyboard')} ${esc(t('code.editorHint'))}${isStatic ? ` · ${esc(t('code.remoteHint'))}` : ''}</div>
        </div>
        ${consoleShell({ id: 'x-console', title: t('code.output'), body: `<pre class="console-body"><span class="muted">${esc(t('code.notRunYet'))}</span></pre>` })}
        <div id="pass-slot" aria-live="polite"></div>
      </div>
    </div>`;

    const editor = createEditor(panel.querySelector('#x-editor'), {
      value: store.loadDraft(draftKey) ?? x.starter,
      onChange: (v) => store.saveDraft(draftKey, v),
      onRun: () => runCode('run'),
    });
    const con = panel.querySelector('#x-console');
    const hintList = panel.querySelector('#hints');
    const hintBtn = panel.querySelector('#hint-btn');

    function drawHints() {
      const n = hintsShown.get(p.id);
      hintList.innerHTML = hints.slice(0, n).map((h) => `<li>${icon('lightbulb')}<span>${rich(h)}</span></li>`).join('');
      hintList.hidden = n === 0;
      if (n >= hints.length) {
        hintBtn.disabled = true;
        hintBtn.innerHTML = `${icon('check')}${esc(t('challenge.noMoreHints'))}`;
      } else {
        hintBtn.disabled = false;
        hintBtn.innerHTML = `${icon('lightbulb')}${esc(t('challenge.showHint', { n: n + 1, m: hints.length }))}`;
      }
    }
    drawHints();
    hintBtn.addEventListener('click', () => {
      hintsShown.set(p.id, Math.min(hints.length, hintsShown.get(p.id) + 1));
      drawHints();
    });

    const buttons = panel.querySelectorAll('.code-toolbar button');
    async function runCode(mode) {
      buttons.forEach((b) => { b.disabled = true; });
      renderRunning(con, mode === 'check' ? t('challenge.checking') : t('code.running'));
      const slot = panel.querySelector('#pass-slot');
      slot.innerHTML = '';
      try {
        const code = editor.getValue();
        if (mode === 'run') {
          const result = await api.run(code);
          renderRunResult(con, result, { expected: x.expectedOutput });
          store.recordRun();
        } else {
          const result = await api.check(p.id, code);
          renderRunResult(con, result, { expected: x.expectedOutput });
          store.recordRun();
          if (result.passed) {
            slot.innerHTML = `<div class="pass-banner">${icon('trophy')}<div><h3>${esc(t('challenge.passed'))}</h3><p>${esc(t('challenge.passedBody'))}</p></div></div>`;
            confetti();
            store.recordExercise(p.id);
            updateHeroState();
          } else if (result.diff) {
            const d = result.diff;
            const content = con.querySelector('.console-content');
            content.insertAdjacentHTML('afterbegin', `<div class="console-section"><span class="console-label">${esc(t('challenge.failed', { line: d.line }))}</span>
              <div class="diff-box">
                <span class="exp"><b>${esc(t('challenge.expectedLine'))}</b><code dir="ltr">${d.expected === null ? esc(t('challenge.missingLine')) : esc(JSON.stringify(d.expected))}</code></span>
                <span class="act"><b>${esc(t('challenge.actualLine'))}</b><code dir="ltr">${d.actual === null ? esc(t('challenge.missingLine')) : esc(JSON.stringify(d.actual))}</code></span>
              </div></div>`);
          }
        }
      } catch (err) {
        renderRunError(con, err);
      } finally {
        buttons.forEach((b) => { b.disabled = false; });
      }
    }

    panel.addEventListener('click', async (e) => {
      const act = e.target.closest('[data-act]');
      if (!act) return;
      if (act.dataset.act === 'run') runCode('run');
      if (act.dataset.act === 'check') runCode('check');
      if (act.dataset.act === 'reset') {
        if (await confirmDialog({ title: t('challenge.resetTitle'), body: t('challenge.resetBody'), confirmLabel: t('code.reset'), danger: true })) {
          editor.setValue(x.starter);
          store.clearDraft(draftKey);
        }
      }
      if (act.dataset.act === 'solution') {
        if (await confirmDialog({ title: t('challenge.solutionTitle'), body: t('challenge.solutionBody'), confirmLabel: t('challenge.solution') })) {
          const { code } = await api.solution(p.id);
          editor.setValue(code);
        }
      }
    });
    requestAnimationFrame(() => editor.refresh());
  },
};
