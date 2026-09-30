'use strict';

const path = require('path');
const express = require('express');
const content = require('./content');
const runner = require('./runner');
const { buildSprite } = require('./icons');

const ROOT = path.join(__dirname, '..');
const MODULES = path.join(ROOT, 'node_modules');

const SECURITY_HEADERS = {
  'Content-Security-Policy': [
    "default-src 'self'",
    "script-src 'self'",
    "style-src 'self' 'unsafe-inline'", // CodeMirror positions elements with inline styles
    "img-src 'self' data:",
    "font-src 'self'",
    "connect-src 'self'",
    "frame-ancestors 'none'",
    "base-uri 'self'",
    "form-action 'self'",
  ].join('; '),
  'X-Content-Type-Options': 'nosniff',
  'Referrer-Policy': 'no-referrer',
  'X-Frame-Options': 'DENY',
};

function shuffle(arr, rand = Math.random) {
  const a = arr.slice();
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(rand() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

function clampInt(value, min, max, fallback) {
  const n = Number.parseInt(value, 10);
  return Number.isFinite(n) ? Math.min(max, Math.max(min, n)) : fallback;
}

/**
 * @param {{ library?: ReturnType<typeof content.loadLibrary>, run?: typeof runner.compileAndRun,
 *           compilerInfo?: typeof runner.compilerInfo }} [deps]
 */
function createApp(deps = {}) {
  const library = deps.library || content.loadLibrary();
  const run = deps.run || runner.compileAndRun;
  const compilerInfo = deps.compilerInfo || runner.compilerInfo;

  const { categories, patterns, byId } = library;
  const summaries = patterns.map(content.summarize);
  const sprite = buildSprite([...categories.map((c) => c.icon), ...patterns.map((p) => p.icon)]);

  // Built-in programs are deterministic, so their results can be cached.
  const builtinHashes = new Set();
  for (const p of patterns) {
    for (const src of [p.sources.example, p.sources.solution, p.sources.exercise]) {
      if (src) builtinHashes.add(runner.hashSource(src));
    }
  }
  const runCache = new Map();

  async function runCached(code) {
    const key = runner.hashSource(code);
    if (!builtinHashes.has(key)) return run(code);
    if (!runCache.has(key)) {
      const pending = run(code).catch((err) => {
        runCache.delete(key);
        throw err;
      });
      runCache.set(key, pending);
    }
    return runCache.get(key);
  }

  const app = express();
  app.disable('x-powered-by');
  app.use((req, res, next) => {
    res.set(SECURITY_HEADERS);
    next();
  });
  app.use(express.json({ limit: '200kb' }));

  // ------------------------------------------------------------------ API
  const api = express.Router();

  api.get('/meta', async (req, res) => {
    res.json({
      categories,
      patternCount: patterns.length,
      questionCount: patterns.reduce((n, p) => n + (p.quiz ? p.quiz.length : 0), 0),
      exerciseCount: patterns.filter((p) => p.exercise).length,
      compiler: await compilerInfo(),
    });
  });

  api.get('/patterns', (req, res) => res.json(summaries));

  api.get('/patterns/:id', (req, res) => {
    const p = byId.get(req.params.id);
    if (!p) return res.status(404).json({ error: 'Pattern not found' });
    res.json(content.lessonPayload(p, library));
  });

  api.get('/patterns/:id/solution', (req, res) => {
    const p = byId.get(req.params.id);
    if (!p) return res.status(404).json({ error: 'Pattern not found' });
    res.json({ code: p.sources.solution });
  });

  api.post('/run', async (req, res, next) => {
    try {
      const code = req.body && req.body.code;
      res.json(await runCached(code));
    } catch (err) {
      next(err);
    }
  });

  api.post('/patterns/:id/check', async (req, res, next) => {
    try {
      const p = byId.get(req.params.id);
      if (!p) return res.status(404).json({ error: 'Pattern not found' });
      const result = await runCached(req.body && req.body.code);
      const diff = result.stage === 'run' && !result.timedOut ? runner.diffOutput(result.stdout, p.exercise.expectedOutput) : null;
      const passed = result.ok && diff === null;
      res.json({ ...result, passed, diff, expected: runner.normalizeOutput(p.exercise.expectedOutput) });
    } catch (err) {
      next(err);
    }
  });

  function pickPatterns(query) {
    const wanted = String(query.categories || '')
      .split(',')
      .map((s) => s.trim())
      .filter((s) => content.CATEGORY_IDS.includes(s));
    return wanted.length ? patterns.filter((p) => wanted.includes(p.category)) : patterns;
  }

  // Mixed quiz: random questions, options shuffled consistently for both languages.
  api.get('/training/quiz', (req, res) => {
    const count = clampInt(req.query.count, 1, 40, 10);
    const pool = [];
    for (const p of pickPatterns(req.query)) {
      (p.quiz || []).forEach((q, i) => pool.push({ p, q, i }));
    }
    const questions = shuffle(pool).slice(0, count).map(({ p, q, i }) => {
      const order = shuffle([0, 1, 2, 3]);
      return {
        key: `${p.id}:${i}`,
        pattern: content.summarize(p),
        question: q.question,
        options: { en: order.map((k) => q.options.en[k]), fa: order.map((k) => q.options.fa[k]) },
        answer: order.indexOf(q.answer),
        explanation: q.explanation,
      };
    });
    res.json(questions);
  });

  // Pattern Match: read a scenario, pick the pattern. Distractors prefer the same category.
  api.get('/training/match', (req, res) => {
    const count = clampInt(req.query.count, 1, 40, 10);
    const chosen = pickPatterns(req.query);
    const pool = [];
    for (const p of chosen) (p.scenarios || []).forEach((s) => pool.push({ p, s }));
    const rounds = shuffle(pool).slice(0, count).map(({ p, s }) => {
      const same = shuffle(patterns.filter((o) => o.id !== p.id && o.category === p.category));
      const other = shuffle(patterns.filter((o) => o.id !== p.id && o.category !== p.category));
      const distractors = [...same.slice(0, 2), ...other].slice(0, 3);
      const options = shuffle([p, ...distractors]).map(content.summarize);
      return { scenario: s, options, answer: options.findIndex((o) => o.id === p.id), pattern: content.summarize(p) };
    });
    res.json(rounds);
  });

  api.use((req, res) => res.status(404).json({ error: 'Not found' }));
  // eslint-disable-next-line no-unused-vars
  api.use((err, req, res, next) => {
    const status = err.status || err.statusCode || 500;
    if (status >= 500 && status !== 503) console.error(err);
    res.status(status).json({ error: status === 500 ? 'Internal server error' : err.message });
  });

  app.use('/api', api);

  // --------------------------------------------------------------- static
  app.get('/icons.svg', (req, res) => {
    res.type('image/svg+xml').set('Cache-Control', 'public, max-age=3600').send(sprite);
  });
  app.use('/vendor/codemirror', express.static(path.join(MODULES, 'codemirror'), { maxAge: '7d' }));
  app.use('/vendor/fonts', express.static(path.join(MODULES, '@fontsource'), { maxAge: '30d' }));
  app.use(express.static(path.join(ROOT, 'public'), { extensions: ['html'] }));

  return app;
}

module.exports = { createApp, shuffle };
