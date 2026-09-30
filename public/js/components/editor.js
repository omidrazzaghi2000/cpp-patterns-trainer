// CodeMirror 5 wrapper + the output console used by the Code, Challenge and Playground views.

import { t } from '../i18n.js';
import { esc, icon } from '../util.js';

/**
 * @param {HTMLElement} host
 * @param {{ value: string, readOnly?: boolean, onChange?: (v: string) => void, onRun?: () => void, label?: string }} opts
 */
export function createEditor(host, { value, readOnly = false, onChange, onRun, label }) {
  if (!window.CodeMirror) {
    // Fallback: plain textarea / pre if CodeMirror failed to load.
    host.innerHTML = readOnly
      ? `<pre class="console-body" style="color:#E0E7FF">${esc(value)}</pre>`
      : `<textarea class="console-body" style="width:100%;min-height:420px;background:#1E1B4B;color:#E0E7FF;border:0" spellcheck="false" aria-label="${esc(label || t('code.editorLabel'))}">${esc(value)}</textarea>`;
    const ta = host.querySelector('textarea');
    if (ta && onChange) ta.addEventListener('input', () => onChange(ta.value));
    return {
      getValue: () => (ta ? ta.value : value),
      setValue: (v) => { if (ta) ta.value = v; },
      highlight: () => {},
      refresh: () => {},
      focus: () => ta && ta.focus(),
    };
  }

  const runKey = () => { if (onRun) onRun(); };
  const cm = window.CodeMirror(host, {
    value,
    mode: 'text/x-c++src',
    theme: 'clay',
    lineNumbers: true,
    readOnly,
    indentUnit: 4,
    tabSize: 4,
    indentWithTabs: false,
    matchBrackets: true,
    autoCloseBrackets: !readOnly,
    styleActiveLine: !readOnly,
    viewportMargin: Infinity,
    direction: 'ltr',
    screenReaderLabel: label || t('code.editorLabel'),
    extraKeys: readOnly
      ? { Tab: false, 'Shift-Tab': false }
      : {
          'Ctrl-Enter': runKey,
          'Cmd-Enter': runKey,
          Esc: (ed) => { ed.state.tabEscape = true; },
          Tab: (ed) => {
            if (ed.state.tabEscape) { ed.state.tabEscape = false; return window.CodeMirror.Pass; }
            if (ed.somethingSelected()) ed.indentSelection('add');
            else ed.replaceSelection(' '.repeat(ed.getOption('indentUnit')), 'end');
            return undefined;
          },
          'Shift-Tab': (ed) => {
            if (ed.state.tabEscape) { ed.state.tabEscape = false; return window.CodeMirror.Pass; }
            ed.indentSelection('subtract');
            return undefined;
          },
        },
  });
  cm.on('keydown', (ed, e) => {
    if (e.key !== 'Escape' && e.key !== 'Tab' && e.key !== 'Shift') ed.state.tabEscape = false;
  });
  if (onChange) cm.on('change', () => onChange(cm.getValue()));

  let marked = [];
  return {
    cm,
    getValue: () => cm.getValue(),
    setValue: (v) => cm.setValue(v),
    focus: () => cm.focus(),
    refresh: () => cm.refresh(),
    /** Highlights 1-based inclusive line range, or clears when range is null. */
    highlight(range) {
      marked.forEach((ln) => cm.removeLineClass(ln, 'background', 'cm-hl'));
      marked = [];
      if (!range) return;
      const [a, b] = range;
      for (let ln = a - 1; ln <= b - 1 && ln < cm.lineCount(); ln++) {
        cm.addLineClass(ln, 'background', 'cm-hl');
        marked.push(ln);
      }
      const top = cm.charCoords({ line: a - 1, ch: 0 }, 'page').top;
      const rect = host.getBoundingClientRect();
      if (top < window.scrollY + 90 || top > window.scrollY + window.innerHeight - 80 || rect.top < 0) {
        window.scrollTo({ top: Math.max(0, top - 160), behavior: window.matchMedia('(prefers-reduced-motion: reduce)').matches ? 'auto' : 'smooth' });
      }
    },
  };
}

export function codeToolbar({ file, buttons }) {
  return `<div class="code-toolbar">
    <span class="file-chip" dir="ltr"><span class="dots" aria-hidden="true"><i></i><i></i><i></i></span>${esc(file)}</span>
    ${buttons}
  </div>`;
}

/** Console shell; `body` is inner HTML for the output area. */
export function consoleShell({ id, title = t('code.output'), status = '', body = '' }) {
  return `<section class="console" id="${id}" aria-live="polite" aria-busy="false">
    <div class="console-head">${icon('terminal')}<span>${esc(title)}</span><span class="spacer"></span><span class="console-status-wrap">${status}</span></div>
    <div class="console-content">${body}</div>
  </section>`;
}

export function statusChip(kind, text, iconName) {
  return `<span class="console-status ${kind}">${iconName ? icon(iconName) : ''}${esc(text)}</span>`;
}

/** Renders a /api/run or /api/check result into a console element. */
export function renderRunResult(consoleEl, result, { expected = null } = {}) {
  const statusWrap = consoleEl.querySelector('.console-status-wrap');
  const content = consoleEl.querySelector('.console-content');
  consoleEl.setAttribute('aria-busy', 'false');
  const timing = `<p class="console-meta">${icon('clock')}${esc(t('code.timing', { c: result.compileMs || 0, r: result.runMs || 0 }))}</p>`;
  const sections = [];

  if (result.stage === 'compile') {
    statusWrap.innerHTML = statusChip('err', t('code.compileError'), 'circle-x');
    sections.push(`<div class="console-section"><span class="console-label">${esc(t('code.compiler'))}</span><pre class="console-body"><span class="err">${esc(result.compileOutput || '')}</span></pre></div>`);
  } else {
    let chip;
    if (result.timedOut) chip = statusChip('err', t('code.timeout'), 'timer');
    else if (result.truncated) chip = statusChip('err', t('code.truncated'), 'triangle-alert');
    else if (result.exitCode !== 0) chip = statusChip('err', t('code.runtimeError', { code: result.exitCode ?? '?' }), 'circle-x');
    else chip = statusChip('ok', t('code.ok'), 'circle-check');
    statusWrap.innerHTML = chip;

    if (result.compileOutput && result.compileOutput.trim()) {
      sections.push(`<div class="console-section"><span class="console-label">${esc(t('code.compiler'))}</span><pre class="console-body"><span class="muted">${esc(result.compileOutput)}</span></pre></div>`);
    }
    sections.push(`<div class="console-section"><span class="console-label">${esc(t('code.stdout'))}</span><pre class="console-body">${esc(result.stdout || '') || '<span class="muted">∅</span>'}</pre></div>`);
    if (result.stderr && result.stderr.trim()) {
      sections.push(`<div class="console-section"><span class="console-label">${esc(t('code.stderr'))}</span><pre class="console-body"><span class="err">${esc(result.stderr)}</span></pre></div>`);
    }
    if (expected !== null && result.exitCode === 0 && !result.timedOut) {
      const same = normalize(result.stdout) === normalize(expected);
      if (same) sections.push(`<div class="console-section"><p class="console-meta ok">${icon('circle-check')}${esc(t('code.matches'))}</p></div>`);
    }
  }
  sections.push(`<div class="console-section">${timing}</div>`);
  content.innerHTML = sections.join('');
}

export function renderRunError(consoleEl, err) {
  consoleEl.setAttribute('aria-busy', 'false');
  consoleEl.querySelector('.console-status-wrap').innerHTML = statusChip('err', err.status === 503 ? t('code.compileError') : 'Error', 'triangle-alert');
  const msg = err.status === 503 ? t('code.noCompiler') : t('code.requestFailed', { msg: err.message });
  consoleEl.querySelector('.console-content').innerHTML = `<pre class="console-body"><span class="err">${esc(msg)}</span></pre>`;
}

export function renderRunning(consoleEl, label = t('code.running')) {
  consoleEl.setAttribute('aria-busy', 'true');
  consoleEl.querySelector('.console-status-wrap').innerHTML = statusChip('info', label, null);
  consoleEl.querySelector('.console-content').innerHTML = `<pre class="console-body"><span class="muted">${icon('loader-circle', 'spin')} ${esc(label)}</span></pre>`;
}

function normalize(s) {
  return String(s ?? '').replace(/\r\n?/g, '\n').split('\n').map((l) => l.replace(/\s+$/, '')).join('\n').replace(/\n+$/, '');
}
