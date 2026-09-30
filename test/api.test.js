'use strict';

const { test, before, after } = require('node:test');
const assert = require('node:assert/strict');
const { createApp } = require('../server/app');
const { loadLibrary, validatePattern } = require('../server/content');
const { compilerInfo, diffOutput, normalizeOutput } = require('../server/runner');

const library = loadLibrary();
let server;
let base;
const runs = [];

// A fake runner keeps API tests fast and independent of the local toolchain.
async function fakeRun(code) {
  runs.push(code);
  if (code.includes('COMPILE_ERROR')) {
    return { ok: false, stage: 'compile', compileOutput: 'main.cpp:1: error', stdout: '', stderr: '', exitCode: 1, timedOut: false, truncated: false, compileMs: 1, runMs: 0 };
  }
  const singleton = library.byId.get('singleton');
  const stdout = code === singleton.sources.solution ? singleton.exercise.expectedOutput : 'user#1\norder#1\nuser#1\n';
  return { ok: true, stage: 'run', compileOutput: '', stdout, stderr: '', exitCode: 0, timedOut: false, truncated: false, compileMs: 1, runMs: 1 };
}

before(async () => {
  const app = createApp({ library, run: fakeRun, compilerInfo: async () => ({ available: true, command: 'fake', standard: 'c++20', version: 'fake 1.0' }) });
  await new Promise((resolve) => {
    server = app.listen(0, '127.0.0.1', resolve);
  });
  base = `http://127.0.0.1:${server.address().port}`;
});

after(() => new Promise((resolve) => server.close(resolve)));

const get = (path) => fetch(base + path);
const post = (path, body) => fetch(base + path, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });

test('content library loads every pattern without schema errors', () => {
  assert.equal(library.patterns.length, 31);
  const known = new Set(library.byId.keys());
  for (const p of library.patterns) {
    const { errors, warnings } = validatePattern(p, { knownIds: known });
    assert.deepEqual(errors, [], `${p.id} has schema errors`);
    assert.deepEqual(warnings, [], `${p.id} has warnings`);
  }
});

test('every category has patterns and ids are unique', () => {
  for (const c of library.categories) {
    assert.ok(library.patterns.some((p) => p.category === c.id), `category ${c.id} is empty`);
  }
  assert.equal(new Set(library.patterns.map((p) => p.id)).size, library.patterns.length);
});

test('GET /api/meta reports counts and compiler', async () => {
  const meta = await (await get('/api/meta')).json();
  assert.equal(meta.patternCount, 31);
  assert.equal(meta.questionCount, 31 * 4);
  assert.equal(meta.exerciseCount, 31);
  assert.equal(meta.categories.length, 4);
  assert.equal(meta.compiler.available, true);
});

test('GET /api/patterns returns bilingual summaries without sources', async () => {
  const list = await (await get('/api/patterns')).json();
  assert.equal(list.length, 31);
  for (const s of list) {
    assert.ok(s.name.en && s.name.fa && s.tagline.en && s.tagline.fa);
    assert.equal(s.sources, undefined);
  }
});

test('GET /api/patterns/:id includes example code and starter but not the solution', async () => {
  const res = await get('/api/patterns/singleton');
  assert.equal(res.status, 200);
  const p = await res.json();
  assert.match(p.example.code, /static AppConfig& instance\(\)/);
  assert.match(p.exercise.starter, /TODO 1/);
  assert.equal(p.sources, undefined);
  assert.equal(JSON.stringify(p).includes(library.byId.get('singleton').sources.solution), false);
  assert.equal(p.prev, null);
  assert.equal(p.next.id, 'factory-method');
});

test('unknown pattern → 404; solution endpoint serves the reference solution', async () => {
  assert.equal((await get('/api/patterns/nope')).status, 404);
  const { code } = await (await get('/api/patterns/singleton/solution')).json();
  assert.equal(code, library.byId.get('singleton').sources.solution);
});

test('POST /api/patterns/:id/check passes the solution and diffs the starter', async () => {
  const s = library.byId.get('singleton');
  const pass = await (await post('/api/patterns/singleton/check', { code: s.sources.solution })).json();
  assert.equal(pass.passed, true);
  assert.equal(pass.diff, null);

  const fail = await (await post('/api/patterns/singleton/check', { code: s.sources.exercise })).json();
  assert.equal(fail.passed, false);
  assert.deepEqual(fail.diff, { line: 2, expected: 'order#2', actual: 'order#1' });

  const broken = await (await post('/api/patterns/singleton/check', { code: 'COMPILE_ERROR' })).json();
  assert.equal(broken.passed, false);
  assert.equal(broken.stage, 'compile');
});

test('built-in programs are cached; user programs are not', async () => {
  const example = library.byId.get('builder').sources.example;
  const before = runs.length;
  await post('/api/run', { code: example });
  await post('/api/run', { code: example });
  assert.equal(runs.length - before, 1);
  await post('/api/run', { code: 'int main() {}' });
  await post('/api/run', { code: 'int main() {}' });
  assert.equal(runs.length - before, 3);
});

test('training quiz shuffles options consistently in both languages', async () => {
  const qs = await (await get('/api/training/quiz?count=40&categories=creational')).json();
  assert.equal(qs.length, 20); // 5 creational patterns × 4 questions
  for (const q of qs) {
    assert.equal(q.pattern.category, 'creational');
    const [id, idx] = q.key.split(':');
    const original = library.byId.get(id).quiz[Number(idx)];
    assert.equal(q.options.en[q.answer], original.options.en[original.answer]);
    assert.equal(q.options.fa[q.answer], original.options.fa[original.answer]);
  }
});

test('pattern match rounds contain the answer exactly once among 4 distinct options', async () => {
  const rounds = await (await get('/api/training/match?count=12')).json();
  assert.equal(rounds.length, 12);
  for (const r of rounds) {
    assert.equal(r.options.length, 4);
    assert.equal(new Set(r.options.map((o) => o.id)).size, 4);
    assert.equal(r.options[r.answer].id, r.pattern.id);
  }
});

test('security headers and static assets are served', async () => {
  const res = await get('/');
  assert.equal(res.status, 200);
  assert.match(res.headers.get('content-security-policy'), /default-src 'self'/);
  assert.equal(res.headers.get('x-content-type-options'), 'nosniff');
  const sprite = await (await get('/icons.svg')).text();
  for (const p of library.patterns) assert.ok(sprite.includes(`id="i-${p.icon}"`), `icon ${p.icon} missing from sprite`);
  assert.equal((await get('/vendor/codemirror/lib/codemirror.js')).status, 200);
  assert.equal((await get('/vendor/fonts/vazirmatn/400.css')).status, 200);
});

test('output normalization ignores trailing whitespace and CRLF only', () => {
  assert.equal(normalizeOutput('a  \r\nb\n\n'), 'a\nb');
  assert.equal(diffOutput('a\nb\n', 'a\nb'), null);
  assert.deepEqual(diffOutput('a\nc', 'a\nb'), { line: 2, expected: 'b', actual: 'c' });
  assert.deepEqual(diffOutput('a', 'a\nb'), { line: 2, expected: 'b', actual: null });
  assert.notEqual(diffOutput(' a', 'a'), null); // leading spaces matter
});

test('real compiler round-trip (skipped when g++ is unavailable)', async (t) => {
  const info = await compilerInfo();
  if (!info.available) return t.skip('no C++ compiler');
  const { compileAndRun } = require('../server/runner');
  const ok = await compileAndRun('#include <iostream>\nint main() { std::cout << "hi\\n"; }');
  assert.equal(ok.ok, true);
  assert.equal(ok.stdout, 'hi\n');
  const bad = await compileAndRun('int main() { return nope; }');
  assert.equal(bad.stage, 'compile');
  const slow = await compileAndRun('int main() { volatile int x = 0; for (;;) ++x; }');
  assert.equal(slow.timedOut, true);
});
