// Backend used by the static build (GitHub Pages): lesson data comes from
// pre-generated JSON files, training rounds are generated in the browser, and
// C++ is compiled and run by Compiler Explorer (https://godbolt.org).

import { shuffle } from './util.js';

const CE_URL = 'https://godbolt.org/api/compiler/g134/compile';
const CE_FLAGS = '-std=c++20 -O0 -Wall -Wextra';
const CATEGORY_IDS = ['creational', 'structural', 'behavioral', 'idioms'];
const ANSI = /\u001b\[[0-9;]*[A-Za-z]/g;

export function normalizeOutput(text) {
  return String(text ?? '')
    .replace(/\r\n?/g, '\n')
    .split('\n')
    .map((line) => line.replace(/\s+$/, ''))
    .join('\n')
    .replace(/\n+$/, '');
}

export function diffOutput(actual, expected) {
  const a = normalizeOutput(actual).split('\n');
  const e = normalizeOutput(expected).split('\n');
  for (let i = 0; i < Math.max(a.length, e.length); i++) {
    if (a[i] !== e[i]) return { line: i + 1, expected: e[i] ?? null, actual: a[i] ?? null };
  }
  return null;
}

const lines = (arr) => (arr || []).map((l) => l.text.replace(ANSI, '').replace(/<source>/g, 'main.cpp')).join('\n');

/** Compiles and runs `code` on Compiler Explorer; returns the same shape as the Node server's /api/run. */
export async function remoteRun(code) {
  if (!code || !code.trim()) throw Object.assign(new Error('Source code is empty.'), { status: 400 });
  const ctrl = new AbortController();
  const timer = setTimeout(() => ctrl.abort(), 45_000);
  const started = performance.now();
  let res;
  try {
    res = await fetch(CE_URL, {
      method: 'POST',
      signal: ctrl.signal,
      headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
      body: JSON.stringify({
        source: code,
        lang: 'c++',
        allowStoreCodeDebug: false,
        options: {
          userArguments: CE_FLAGS,
          executeParameters: { args: [], stdin: '' },
          compilerOptions: { executorRequest: true, skipAsm: true },
          filters: { execute: true },
          tools: [],
          libraries: [],
        },
      }),
    });
  } catch (err) {
    throw Object.assign(new Error(err.name === 'AbortError' ? 'Compiler Explorer did not answer in time' : 'Compiler Explorer (godbolt.org) is unreachable'), { status: 0 });
  } finally {
    clearTimeout(timer);
  }
  if (!res.ok) throw Object.assign(new Error(`Compiler Explorer answered HTTP ${res.status}${res.status === 429 ? ' (too many requests — wait a moment)' : ''}`), { status: res.status });
  const j = await res.json();
  const total = Math.round(performance.now() - started);
  const build = j.buildResult || { code: j.code, stderr: j.stderr, stdout: [] };

  if (build.code !== 0 || !j.didExecute) {
    return {
      ok: false, stage: 'compile', compileOutput: lines(build.stderr) || lines(build.stdout) || 'Compilation failed.',
      stdout: '', stderr: '', exitCode: build.code, timedOut: Boolean(j.timedOut), truncated: false,
      compileMs: total, runMs: 0,
    };
  }
  const stdout = j.stdout && j.stdout.length ? `${lines(j.stdout)}\n` : '';
  return {
    ok: j.code === 0 && !j.timedOut && !j.truncated,
    stage: 'run',
    compileOutput: lines(build.stderr),
    stdout,
    stderr: lines(j.stderr),
    exitCode: j.code,
    timedOut: Boolean(j.timedOut),
    truncated: Boolean(j.truncated),
    compileMs: Math.max(0, total - (j.execTime || 0)),
    runMs: Number(j.execTime) || 0,
  };
}

function pick(training, categories) {
  const wanted = String(categories || '').split(',').filter((c) => CATEGORY_IDS.includes(c));
  return wanted.length ? training.filter((t) => wanted.includes(t.pattern.category)) : training;
}

const clamp = (v, fallback) => Math.min(40, Math.max(1, Number.parseInt(v, 10) || fallback));

/** Same behaviour as GET /api/training/quiz on the Node server. */
export function makeQuiz(training, params) {
  const pool = [];
  for (const t of pick(training, params.categories)) t.quiz.forEach((q, i) => pool.push({ t, q, i }));
  return shuffle(pool).slice(0, clamp(params.count, 10)).map(({ t, q, i }) => {
    const order = shuffle([0, 1, 2, 3]);
    return {
      key: `${t.pattern.id}:${i}`,
      pattern: t.pattern,
      question: q.question,
      options: { en: order.map((k) => q.options.en[k]), fa: order.map((k) => q.options.fa[k]) },
      answer: order.indexOf(q.answer),
      explanation: q.explanation,
    };
  });
}

/** Same behaviour as GET /api/training/match on the Node server. */
export function makeMatch(training, params) {
  const all = training.map((t) => t.pattern);
  const pool = [];
  for (const t of pick(training, params.categories)) t.scenarios.forEach((s) => pool.push({ p: t.pattern, s }));
  return shuffle(pool).slice(0, clamp(params.count, 10)).map(({ p, s }) => {
    const same = shuffle(all.filter((o) => o.id !== p.id && o.category === p.category));
    const other = shuffle(all.filter((o) => o.id !== p.id && o.category !== p.category));
    const options = shuffle([p, ...[...same.slice(0, 2), ...other].slice(0, 3)]);
    return { scenario: s, options, answer: options.findIndex((o) => o.id === p.id), pattern: p };
  });
}
