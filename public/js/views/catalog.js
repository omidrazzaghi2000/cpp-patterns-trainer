import { t, L, other, num } from '../i18n.js';
import { store } from '../store.js';
import { esc, icon, rich, tx, difficultyDots, debounce } from '../util.js';

export function patternCard(p) {
  const s = store.get();
  const q = s.quiz[p.id];
  const chips = [];
  if (s.learned[p.id]) chips.push(`<span class="chip chip--success">${icon('bookmark-check')}${esc(t('catalog.learned'))}</span>`);
  if (q) chips.push(`<span class="chip ${q.best === q.total ? 'chip--success' : 'chip--warn'}">${icon('list-checks')}${esc(t('catalog.quiz', { n: q.best, m: q.total }))}</span>`);
  if (s.exercises[p.id]) chips.push(`<span class="chip chip--success">${icon('puzzle')}${esc(t('catalog.solved'))}</span>`);
  return `<a class="pcard" data-cat="${esc(p.category)}" href="#/p/${esc(p.id)}">
    <div class="pcard-top"><span class="icon-tile">${icon(p.icon)}</span>${difficultyDots(p.difficulty)}</div>
    <h3>${rich(L(p.name))}</h3>
    <p class="alt-name" lang="${document.documentElement.lang === 'fa' ? 'en' : 'fa'}">${esc(other(p.name))}</p>
    <p class="tagline">${rich(L(p.tagline))}</p>
    ${chips.length ? `<div class="pcard-status">${chips.join('')}</div>` : ''}
  </a>`;
}

function matches(p, q) {
  if (!q) return true;
  const hay = [p.id, p.name.en, p.name.fa, p.tagline.en, p.tagline.fa, p.intent.en, p.intent.fa].join(' ').toLowerCase();
  return q.toLowerCase().split(/\s+/).filter(Boolean).every((w) => hay.includes(w));
}

export function render(ctx) {
  const { root, patterns, categories, query } = ctx;
  ctx.setTitle(t('catalog.title'));
  const state = {
    cat: categories.some((c) => c.id === query.get('cat')) ? query.get('cat') : 'all',
    q: query.get('q') || '',
    d: ['1', '2', '3'].includes(query.get('d')) ? query.get('d') : '',
  };

  root.innerHTML = `
    <header class="page-head">
      <h1>${tx('catalog.title')}</h1>
      <p>${tx('catalog.sub')}</p>
    </header>
    <div class="clay toolbar">
      <label class="search">
        <span class="sr-only">${esc(t('catalog.searchLabel'))}</span>
        ${icon('search')}
        <input class="input" type="search" id="q" placeholder="${esc(t('catalog.search'))}" value="${esc(state.q)}" autocomplete="off">
      </label>
      <div class="btn-row">
        <div class="filter-chips" role="group" aria-label="${esc(t('catalog.categoryFilter'))}">
          <button type="button" class="chip-toggle" data-cat-filter="all" aria-pressed="${state.cat === 'all'}">${esc(t('common.all'))}</button>
          ${categories.map((c) => `<button type="button" class="chip-toggle" data-cat="${esc(c.id)}" data-cat-filter="${esc(c.id)}" aria-pressed="${state.cat === c.id}"><span class="dot" aria-hidden="true"></span>${rich(L(c.name))}</button>`).join('')}
        </div>
        <label class="select-wrap">
          <span class="sr-only">${esc(t('diff.label'))}</span>
          <select class="input" id="d">
            <option value="">${esc(t('catalog.anyDifficulty'))}</option>
            ${[1, 2, 3].map((d) => `<option value="${d}" ${state.d === String(d) ? 'selected' : ''}>${esc(t(`diff.${d}`))}</option>`).join('')}
          </select>
          ${icon('chevron-down')}
        </label>
      </div>
    </div>
    <p class="result-count" id="count" aria-live="polite"></p>
    <div id="results"></div>`;

  const results = root.querySelector('#results');
  const count = root.querySelector('#count');

  function syncUrl() {
    const qs = new URLSearchParams();
    if (state.cat !== 'all') qs.set('cat', state.cat);
    if (state.q) qs.set('q', state.q);
    if (state.d) qs.set('d', state.d);
    const s = qs.toString();
    history.replaceState(null, '', `#/patterns${s ? `?${s}` : ''}`);
  }

  function draw() {
    const list = patterns.filter((p) => (state.cat === 'all' || p.category === state.cat) && (!state.d || String(p.difficulty) === state.d) && matches(p, state.q));
    count.textContent = t('catalog.count', { n: list.length });
    if (!list.length) {
      results.innerHTML = `<div class="clay empty-state" style="min-height:240px;padding:32px">
        ${icon('search', 'icon--xl')}
        <p>${esc(t('catalog.none'))}</p>
        <button type="button" class="btn btn--primary" id="clear">${icon('rotate-ccw')}${esc(t('catalog.clear'))}</button>
      </div>`;
      return;
    }
    results.innerHTML = categories
      .map((c) => {
        const group = list.filter((p) => p.category === c.id);
        if (!group.length) return '';
        return `<section class="cat-section" data-cat="${esc(c.id)}" aria-labelledby="h-${esc(c.id)}">
          <h2 class="cat-heading" id="h-${esc(c.id)}"><span class="cat-badge" aria-hidden="true">${icon(c.icon)}</span>${rich(L(c.title))}<span class="count">${num(group.length)}</span></h2>
          <div class="pattern-grid">${group.map(patternCard).join('')}</div>
        </section>`;
      })
      .join('');
  }

  root.addEventListener('click', (e) => {
    const chip = e.target.closest('[data-cat-filter]');
    if (chip) {
      state.cat = chip.dataset.catFilter;
      root.querySelectorAll('[data-cat-filter]').forEach((b) => b.setAttribute('aria-pressed', String(b === chip)));
      syncUrl();
      draw();
    }
    if (e.target.closest('#clear')) {
      state.cat = 'all';
      state.q = '';
      state.d = '';
      root.querySelector('#q').value = '';
      root.querySelector('#d').value = '';
      root.querySelectorAll('[data-cat-filter]').forEach((b) => b.setAttribute('aria-pressed', String(b.dataset.catFilter === 'all')));
      syncUrl();
      draw();
    }
  });
  const onSearch = debounce(() => {
    state.q = root.querySelector('#q').value.trim();
    syncUrl();
    draw();
  }, 150);
  root.querySelector('#q').addEventListener('input', onSearch);
  root.querySelector('#d').addEventListener('change', (e) => {
    state.d = e.target.value;
    syncUrl();
    draw();
  });

  draw();
}
