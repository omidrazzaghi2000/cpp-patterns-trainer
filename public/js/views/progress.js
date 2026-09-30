import { t, L, num } from '../i18n.js';
import { store, levelInfo, badgeList } from '../store.js';
import { esc, icon, rich, tx, toast, confirmDialog } from '../util.js';

export function render(ctx) {
  const { root } = ctx;
  ctx.setTitle(t('progress.title'));
  draw(ctx);

  root.addEventListener('click', async (e) => {
    const act = e.target.closest('[data-act]');
    if (!act) return;
    if (act.dataset.act === 'export') {
      const blob = new Blob([store.export()], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `cpp-patterns-progress-${new Date().toISOString().slice(0, 10)}.json`;
      document.body.appendChild(a);
      a.click();
      a.remove();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
    }
    if (act.dataset.act === 'import') root.querySelector('#import-file').click();
    if (act.dataset.act === 'reset') {
      const ok = await confirmDialog({ title: t('progress.resetTitle'), body: t('progress.resetBody'), confirmLabel: t('progress.reset'), danger: true });
      if (ok) {
        store.reset();
        toast(t('progress.resetDone'), { iconName: 'trash-2', tone: 'red' });
        draw(ctx);
      }
    }
  });
  root.addEventListener('change', async (e) => {
    if (e.target.id !== 'import-file' || !e.target.files[0]) return;
    try {
      store.import(await e.target.files[0].text());
      toast(t('progress.imported'), { iconName: 'upload', tone: 'green' });
      draw(ctx);
    } catch {
      toast(t('progress.importFailed'), { iconName: 'triangle-alert', tone: 'red' });
    }
    e.target.value = '';
  });
}

function draw(ctx) {
  const { root, patterns, categories } = ctx;
  const s = store.get();
  const lvl = levelInfo();
  const levels = t('levels');
  const learned = patterns.filter((p) => s.learned[p.id]).length;
  const perfect = Object.values(s.quiz).filter((q) => q.best === q.total).length;
  const solved = patterns.filter((p) => s.exercises[p.id]).length;
  const badges = badgeList();

  root.innerHTML = `
    <header class="page-head"><h1>${tx('progress.title')}</h1><p>${tx('progress.sub')}</p></header>

    <section class="block level-block" aria-labelledby="lvl-title">
      <div class="level-badge" aria-hidden="true"><div><small>${esc(t('level'))}</small><strong>${num(lvl.level)}</strong></div></div>
      <div>
        <span class="chip chip--light">${icon('star')}${num(lvl.xp)} ${esc(t('xp'))}</span>
        <h2 id="lvl-title" style="margin-top:12px">${esc(levels[lvl.level - 1] || levels[levels.length - 1])}</h2>
        <div class="meter" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${lvl.pct}" aria-label="${esc(t('progress.levelOf', { n: lvl.level }))}"><span style="--value:${lvl.pct}%"></span></div>
        <p style="margin:0;font-weight:700">${esc(lvl.max ? t('progress.maxLevel') : t('progress.toNext', { n: lvl.toNext, l: lvl.level + 1 }))}</p>
      </div>
    </section>

    <div class="stats-row">
      <div class="clay stat tone-violet"><span class="stat-icon">${icon('bookmark-check')}</span><span class="stat-value">${num(learned)}<small style="font-size:.5em;color:var(--color-muted-foreground)"> / ${num(patterns.length)}</small></span><span class="stat-label">${esc(t('progress.stats.learned'))}</span></div>
      <div class="clay stat tone-amber"><span class="stat-icon">${icon('medal')}</span><span class="stat-value">${num(perfect)}</span><span class="stat-label">${esc(t('progress.stats.perfect'))}</span></div>
      <div class="clay stat tone-teal"><span class="stat-icon">${icon('puzzle')}</span><span class="stat-value">${num(solved)}<small style="font-size:.5em;color:var(--color-muted-foreground)"> / ${num(patterns.length)}</small></span><span class="stat-label">${esc(t('progress.stats.solved'))}</span></div>
      <div class="clay stat tone-red"><span class="stat-icon">${icon('flame')}</span><span class="stat-value">${num(s.streak.count)}</span><span class="stat-label">${esc(t('progress.stats.streak'))}</span></div>
    </div>

    <section class="section card" aria-labelledby="bycat-title">
      <h2 id="bycat-title">${icon('chart-no-axes-column')}${esc(t('progress.byCategory'))}</h2>
      <div class="cat-progress">
        ${categories.map((c) => {
          const list = patterns.filter((p) => p.category === c.id);
          const score = list.reduce((n, p) => n + (s.learned[p.id] ? 1 : 0) + (s.quiz[p.id] && s.quiz[p.id].best === s.quiz[p.id].total ? 1 : 0) + (s.exercises[p.id] ? 1 : 0), 0);
          const max = list.length * 3;
          const pct = max ? Math.round((score / max) * 100) : 0;
          return `<div class="cat-progress-row" data-cat="${esc(c.id)}">
            <span class="cat-badge" aria-hidden="true">${icon(c.icon)}</span>
            <div>
              <div class="row-top"><span>${rich(L(c.title))}</span><span>${num(pct)}${document.documentElement.lang === 'fa' ? '٪' : '%'}</span></div>
              <div class="meter meter--light" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${pct}" aria-label="${esc(L(c.title))}"><span style="--value:${pct}%"></span></div>
            </div>
          </div>`;
        }).join('')}
      </div>
    </section>

    <section class="section" aria-labelledby="badges-title">
      <div class="section-head"><h2 class="section-title" id="badges-title">${esc(t('progress.badges'))}</h2><span class="chip">${num(badges.filter((b) => b.unlocked).length)} / ${num(badges.length)}</span></div>
      <div class="badge-grid">
        ${badges.map((b) => {
          const [name, desc] = t(`badges.${b.id}`);
          return `<div class="badge tone-${b.tone} ${b.unlocked ? '' : 'locked'}">
            <span class="badge-icon" aria-hidden="true">${icon(b.unlocked ? b.icon : 'lock')}</span>
            <strong>${rich(name)}</strong>
            <small>${rich(desc)}</small>
            <span class="state">${icon(b.unlocked ? 'circle-check' : 'lock')}${esc(t(b.unlocked ? 'progress.unlocked' : 'progress.locked'))}</span>
          </div>`;
        }).join('')}
      </div>
    </section>

    <section class="section card" aria-labelledby="table-title">
      <h2 id="table-title">${icon('list-checks')}${esc(t('progress.table'))}</h2>
      <div class="table-wrap">
        <table class="ptable">
          <thead><tr><th scope="col">${esc(t('progress.th.pattern'))}</th><th scope="col">${esc(t('progress.th.learned'))}</th><th scope="col">${esc(t('progress.th.quiz'))}</th><th scope="col">${esc(t('progress.th.challenge'))}</th></tr></thead>
          <tbody>${patterns.map((p) => {
            const q = s.quiz[p.id];
            return `<tr data-cat="${esc(p.category)}">
              <td><a href="#/p/${esc(p.id)}"><span class="dotcat" aria-hidden="true"></span>${rich(L(p.name))}</a></td>
              <td>${s.learned[p.id] ? `<span class="chip chip--success">${icon('check')}${esc(t('catalog.learned'))}</span>` : '<span aria-label="—">—</span>'}</td>
              <td>${q ? `<span class="chip ${q.best === q.total ? 'chip--success' : 'chip--warn'}">${num(q.best)}/${num(q.total)}</span>` : '—'}</td>
              <td>${s.exercises[p.id] ? `<span class="chip chip--success">${icon('check')}${esc(t('catalog.solved'))}</span>` : `<a href="#/p/${esc(p.id)}/challenge" style="font-weight:700">${icon('puzzle')}${esc(t('pattern.tabs.challenge'))}</a>`}</td>
            </tr>`;
          }).join('')}</tbody>
        </table>
      </div>
    </section>

    <section class="section card" aria-labelledby="data-title">
      <h2 id="data-title">${icon('download')}${esc(t('progress.data'))}</h2>
      <p style="color:var(--color-muted-foreground)">${esc(t('progress.dataSub'))}</p>
      <div class="btn-row">
        <button type="button" class="btn btn--primary" data-act="export">${icon('download')}${esc(t('progress.export'))}</button>
        <button type="button" class="btn btn--light" data-act="import">${icon('upload')}${esc(t('progress.import'))}</button>
        <button type="button" class="btn btn--danger" data-act="reset">${icon('trash-2')}${esc(t('progress.reset'))}</button>
        <input type="file" id="import-file" accept="application/json,.json" hidden>
      </div>
    </section>`;
}
