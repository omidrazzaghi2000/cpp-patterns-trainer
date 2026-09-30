import { t, L, num } from '../i18n.js';
import { store } from '../store.js';
import { esc, icon, rich, tx } from '../util.js';

const LOOP_ICONS = ['book-open', 'network', 'play', 'list-checks', 'puzzle'];

export function render(ctx) {
  const { root, meta, patterns, categories, catById } = ctx;
  ctx.setTitle('');
  const s = store.get();
  const learned = patterns.filter((p) => s.learned[p.id]).length;
  const next = patterns.find((p) => !s.learned[p.id]);
  const started = learned > 0 || Object.keys(s.quiz).length > 0;
  const [before, after] = t('home.title').split('{hl}');

  const statTiles = [
    { tone: 'violet', icon: 'blocks', value: meta.patternCount, label: t('home.stats.patterns') },
    { tone: 'teal', icon: 'list-checks', value: meta.questionCount, label: t('home.stats.questions') },
    { tone: 'pink', icon: 'puzzle', value: meta.exerciseCount, label: t('home.stats.challenges') },
    { tone: 'amber', icon: 'graduation-cap', value: learned, label: t('home.stats.learned') },
  ];

  const nextCat = next ? catById.get(next.category) : null;

  root.innerHTML = `
    <section class="hero block" aria-labelledby="hero-title">
      <div class="hero-copy">
        <span class="chip chip--light">${icon('sparkles')}${tx('home.eyebrow', { n: patterns.length })}</span>
        <h1 class="hero-title" id="hero-title">${rich(before)}<span class="hl">${tx('home.titleHl')}</span>${rich(after || '')}</h1>
        <p class="hero-sub">${tx('home.sub')}</p>
        <div class="hero-cta">
          <a class="btn btn--accent btn--lg" href="#/p/${esc((next || patterns[0]).id)}">${icon('play')}${esc(t(started ? 'home.continue' : 'home.start'))}</a>
          <a class="btn btn--light btn--lg" href="#/training">${icon('dumbbell')}${esc(t('home.train'))}</a>
        </div>
      </div>
      <div class="hero-art" aria-hidden="true">
        <span class="shape s1"></span><span class="shape s2"></span><span class="shape s3"></span><span class="shape s4"></span>
        <div class="code-card" dir="ltr">
          <div class="dots"><span></span><span></span><span></span></div>
<pre><span class="tok-c">// learn one block at a time</span>
<span class="tok-k">class</span> <span class="tok-t">Pattern</span> {
<span class="tok-k">public</span>:
  <span class="tok-k">virtual</span> <span class="tok-t">void</span> <span class="tok-f">learn</span>() = <span class="tok-f">0</span>;
  <span class="tok-k">virtual</span> ~<span class="tok-t">Pattern</span>() = <span class="tok-k">default</span>;
};

<span class="tok-k">auto</span> you = std::<span class="tok-f">make_unique</span>&lt;<span class="tok-t">Expert</span>&gt;();</pre>
        </div>
      </div>
    </section>

    <div class="stats-row">
      ${statTiles.map((st) => `<div class="clay stat tone-${st.tone}">
        <span class="stat-icon">${icon(st.icon)}</span>
        <span class="stat-value">${num(st.value)}</span>
        <span class="stat-label">${esc(st.label)}</span>
      </div>`).join('')}
    </div>

    ${started ? `<section class="section" aria-labelledby="next-title">
      ${next ? `<a class="block continue-block" data-cat="${esc(next.category)}" href="#/p/${esc(next.id)}" style="text-decoration:none">
        <span class="big-icon">${icon(next.icon)}</span>
        <div>
          <span class="chip chip--light">${icon('route')}${esc(t('home.upNext'))} · ${esc(L(nextCat.name))}</span>
          <h3 id="next-title" style="margin-top:12px">${rich(L(next.name))}</h3>
          <p>${rich(L(next.tagline))}</p>
        </div>
        <span class="btn btn--light">${esc(t('home.continue'))}${icon('arrow-right', 'flip-rtl')}</span>
      </a>` : `<div class="block block--green continue-block"><span class="big-icon" style="color:var(--color-success)">${icon('trophy')}</span><div><h3 id="next-title">${tx('home.allDone')}</h3></div><a class="btn btn--light" href="#/training">${esc(t('home.train'))}</a></div>`}
    </section>` : ''}

    <section class="section" aria-labelledby="cats-title">
      <div class="section-head">
        <div>
          <h2 class="section-title" id="cats-title">${tx('home.catsTitle')}</h2>
          <p>${tx('home.catsSub')}</p>
        </div>
      </div>
      <div class="cat-grid">
        ${categories.map((c) => {
          const list = patterns.filter((p) => p.category === c.id);
          const done = list.filter((p) => s.learned[p.id]).length;
          const pct = list.length ? Math.round((done / list.length) * 100) : 0;
          return `<a class="block cat-block" data-cat="${esc(c.id)}" href="#/patterns?cat=${esc(c.id)}">
            <div class="cat-top">
              <span class="cat-icon">${icon(c.icon)}</span>
              <span class="cat-count" aria-hidden="true">${num(list.length)}</span>
            </div>
            <h3>${rich(L(c.title))}</h3>
            <p>${rich(L(c.description))}</p>
            <div class="cat-foot">
              <span class="meter" role="progressbar" aria-valuemin="0" aria-valuemax="${list.length}" aria-valuenow="${done}" aria-label="${esc(t('home.learnedOf', { n: done, m: list.length }))}"><span style="--value:${pct}%"></span></span>
              <span class="meter-label">${esc(t('home.learnedOf', { n: done, m: list.length }))}</span>
            </div>
          </a>`;
        }).join('')}
      </div>
    </section>

    <section class="section" aria-labelledby="loop-title">
      <div class="section-head">
        <div>
          <h2 class="section-title" id="loop-title">${tx('home.loopTitle')}</h2>
          <p>${tx('home.loopSub')}</p>
        </div>
      </div>
      <ol class="loop-grid" style="list-style:none;padding:0;margin:0">
        ${t('home.loop').map(([title, body], i) => `<li class="clay loop-step">
          <span class="num" aria-hidden="true">${num(i + 1)}</span>
          <h3>${icon(LOOP_ICONS[i])}${rich(title)}</h3>
          <p>${rich(body)}</p>
        </li>`).join('')}
      </ol>
    </section>`;
}
