import { t, L } from '../i18n.js';
import { api } from '../api.js';
import { store } from '../store.js';
import { esc, icon, rich, tx } from '../util.js';
import { createEditor, codeToolbar, consoleShell, renderRunResult, renderRunError, renderRunning } from '../components/editor.js';

const BLANK = `#include <iostream>
#include <string>

int main() {
    std::string name = "patterns";
    std::cout << "Hello, " << name << "!\\n";
}
`;
const DRAFT = 'playground';

export function render(ctx) {
  const { root, patterns, categories, meta } = ctx;
  ctx.setTitle(t('playground.title'));
  const compiler = meta.compiler;

  root.innerHTML = `
    <header class="page-head">
      <h1>${tx('playground.title')}</h1>
      <p>${tx('playground.sub')}</p>
    </header>
    ${compiler.available ? '' : `<div class="notice" role="alert">${icon('triangle-alert')}<span>${tx('code.noCompiler')}</span></div>`}
    <div class="play-toolbar">
      <label class="select-wrap">
        <span class="sr-only">${esc(t('playground.load'))}</span>
        <select class="input" id="loader">
          <option value="">${esc(t('playground.load'))}…</option>
          <option value="__blank">${esc(t('playground.blank'))}</option>
          ${categories.map((c) => `<optgroup label="${esc(L(c.title))}">${patterns.filter((p) => p.category === c.id).map((p) => `<option value="${esc(p.id)}">${esc(L(p.name))}</option>`).join('')}</optgroup>`).join('')}
        </select>
        ${icon('chevron-down')}
      </label>
      <span class="chip" dir="ltr">${icon('cpu')}${esc(compiler.available ? `${compiler.command} -std=${compiler.standard}` : 'no compiler')}</span>
    </div>
    <div class="play-layout">
      <div class="code-panel editor">
        ${codeToolbar({
          file: 'main.cpp',
          buttons: `<button type="button" class="btn btn--accent btn--sm" data-act="run" ${compiler.available ? '' : 'disabled'}>${icon('play')}<span>${esc(t('code.run'))}</span></button>`,
        })}
        <div class="cm-host" id="pg-editor"></div>
        <div class="editor-hint">${icon('keyboard')} ${esc(t('code.editorHint'))}${compiler.remote ? ` · ${esc(t('code.remoteHint'))}` : ''}</div>
      </div>
      ${consoleShell({ id: 'pg-console', body: `<pre class="console-body"><span class="muted">${esc(t('code.notRunYet'))}</span></pre>` })}
    </div>
    <p style="margin-top:16px;color:var(--color-muted-foreground);font-size:.92rem;display:flex;gap:8px;align-items:flex-start">${icon('info')}<span>${tx(compiler.remote ? 'playground.remote' : 'playground.localOnly')}</span></p>`;

  const editor = createEditor(root.querySelector('#pg-editor'), {
    value: store.loadDraft(DRAFT) ?? BLANK,
    onChange: (v) => store.saveDraft(DRAFT, v),
    onRun: () => run(),
  });
  const con = root.querySelector('#pg-console');
  const runBtn = root.querySelector('[data-act="run"]');

  async function run() {
    if (!compiler.available || runBtn.disabled) return;
    runBtn.disabled = true;
    renderRunning(con);
    try {
      renderRunResult(con, await api.run(editor.getValue()));
      store.recordRun();
    } catch (err) {
      renderRunError(con, err);
    } finally {
      runBtn.disabled = false;
    }
  }

  runBtn.addEventListener('click', run);
  root.querySelector('#loader').addEventListener('change', async (e) => {
    const id = e.target.value;
    e.target.value = '';
    if (!id) return;
    const code = id === '__blank' ? BLANK : (await api.pattern(id)).example.code;
    editor.setValue(code);
    store.saveDraft(DRAFT, code);
    editor.focus();
  });
  requestAnimationFrame(() => editor.refresh());
}
