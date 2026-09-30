// Multiple-choice quiz. State lives in a caller-owned object so the quiz
// survives re-renders (e.g. switching language mid-quiz).

import { L, t, num } from '../i18n.js';
import { esc, icon, rich, confetti } from '../util.js';

/**
 * @param {HTMLElement} root
 * @param {{ questions: Array, state: {index:number, picks:number[], reported?:boolean},
 *           onComplete?: (score:number, total:number) => void, onRestart?: () => void,
 *           showPattern?: boolean, category?: string }} opts
 */
export function mountQuiz(root, opts) {
  const { questions, state } = opts;

  function score() {
    return state.picks.reduce((n, p, i) => n + (p === questions[i].answer ? 1 : 0), 0);
  }

  function render(focusHeading = false) {
    if (state.index >= questions.length) return renderResults();
    const i = state.index;
    const q = questions[i];
    const picked = state.picks[i];
    const answered = picked !== undefined;
    const letters = t('quiz.letters');
    const pct = Math.round(((i + (answered ? 1 : 0)) / questions.length) * 100);

    const options = L(q.options)
      .map((opt, k) => {
        let cls = '';
        let mark = '';
        if (answered) {
          if (k === q.answer) { cls = 'correct'; mark = `${icon('circle-check')}${esc(t('quiz.correctMark'))}`; }
          else if (k === picked) { cls = 'wrong'; mark = `${icon('circle-x')}${esc(t('quiz.wrongMark'))}`; }
          else cls = 'dim';
        }
        return `<button type="button" class="option ${cls}" data-k="${k}" ${answered ? 'disabled' : ''}>
          <span class="letter" aria-hidden="true">${esc(letters[k])}</span>
          <span>${rich(opt)}</span>
          <span class="mark">${mark}</span>
        </button>`;
      })
      .join('');

    const good = answered && picked === q.answer;
    const feedback = answered
      ? `<div class="feedback ${good ? 'good' : 'bad'}">
          <h4>${icon(good ? 'party-popper' : 'lightbulb')}${esc(t(good ? 'quiz.correct' : 'quiz.wrong'))}</h4>
          <p>${rich(L(q.explanation))}</p>
        </div>
        <div class="quiz-actions">
          <button type="button" class="btn btn--primary" data-act="next">${esc(t(i === questions.length - 1 ? 'quiz.results' : 'quiz.next'))}${icon('arrow-right', 'flip-rtl')}</button>
        </div>`
      : '';

    root.innerHTML = `<div class="quiz">
      <div class="quiz-top">
        <div class="meter meter--light" role="progressbar" aria-valuemin="0" aria-valuemax="${questions.length}" aria-valuenow="${i + (answered ? 1 : 0)}" aria-label="${esc(t('quiz.progress', { n: i + 1, m: questions.length }))}"><span style="--value:${pct}%"></span></div>
        <span class="quiz-count">${esc(t('quiz.progress', { n: i + 1, m: questions.length }))}</span>
      </div>
      <article class="card q-card">
        ${opts.showPattern && q.pattern ? `<div class="q-pattern" data-cat="${esc(q.pattern.category)}"><span class="chip chip--cat">${icon(q.pattern.icon)}${esc(L(q.pattern.name))}</span></div>` : ''}
        <h3 class="q-text" tabindex="-1">${rich(L(q.question))}</h3>
        <div class="options" role="group" aria-label="${esc(t('quiz.yourAnswer'))}">${options}</div>
        <div aria-live="polite">${feedback}</div>
      </article>
    </div>`;

    if (answered) root.querySelector('[data-act="next"]').focus();
    else if (focusHeading) root.querySelector('.q-text').focus();
  }

  function renderResults() {
    const s = score();
    const total = questions.length;
    const pct = Math.round((s / total) * 100);
    if (!state.reported) {
      state.reported = true;
      if (opts.onComplete) opts.onComplete(s, total);
      if (s === total) confetti();
    }
    const msg = s === total ? t('quiz.perfect') : pct >= 60 ? t('quiz.great') : t('quiz.keep');
    const mistakes = questions
      .map((q, i) => ({ q, pick: state.picks[i] }))
      .filter(({ q, pick }) => pick !== q.answer);

    root.innerHTML = `<div class="quiz stack">
      <section class="block result-block ${opts.category ? '' : 'block--indigo'}" ${opts.category ? `data-cat="${esc(opts.category)}"` : ''}>
        <div class="score-ring" style="--p:${pct}"><strong>${num(s)}/${num(total)}</strong></div>
        <h2 tabindex="-1">${esc(msg)}</h2>
        <p>${esc(t('quiz.score', { n: s, m: total }))}</p>
        <div class="btn-row" style="justify-content:center">
          <button type="button" class="btn btn--light" data-act="again">${icon('rotate-ccw')}${esc(t('quiz.again'))}</button>
          ${opts.extraResultButtons || ''}
        </div>
      </section>
      ${mistakes.length ? `<section class="card">
        <h3>${icon('book-open')}${esc(t('quiz.review'))}</h3>
        <ul class="review-list">${mistakes.map(({ q }) => `<li>
          <div class="q">${rich(L(q.question))}</div>
          <div class="a">${icon('circle-check')}<span>${rich(L(q.options)[q.answer])}</span></div>
          <p style="margin:6px 0 0">${rich(L(q.explanation))}</p>
        </li>`).join('')}</ul>
      </section>` : ''}
    </div>`;
    root.querySelector('h2').focus();
  }

  root.onclick = (e) => {
    const opt = e.target.closest('.option[data-k]');
    if (opt && !opt.disabled && state.picks[state.index] === undefined) {
      state.picks[state.index] = Number(opt.dataset.k);
      if (opts.onAnswer) opts.onAnswer(state.picks[state.index] === questions[state.index].answer);
      render();
      return;
    }
    const act = e.target.closest('[data-act]');
    if (!act) return;
    if (act.dataset.act === 'next') {
      state.index += 1;
      render(true);
    } else if (act.dataset.act === 'again') {
      state.index = 0;
      state.picks = [];
      state.reported = false;
      if (opts.onRestart) opts.onRestart();
      else render(true);
    }
  };

  root.onkeydown = (e) => {
    if (e.target.closest('input, textarea, select') || e.altKey || e.ctrlKey || e.metaKey) return;
    if (state.index >= questions.length || state.picks[state.index] !== undefined) return;
    const k = ['1', '2', '3', '4'].indexOf(e.key);
    if (k >= 0) {
      const btn = root.querySelector(`.option[data-k="${k}"]`);
      if (btn) btn.click();
    }
  };

  render();
  return { render, score };
}
