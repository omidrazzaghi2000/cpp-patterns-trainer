// Data access for the app. Two modes, chosen by <meta name="app-mode">:
//   server – the Node/Express API (local g++ compiles the code)
//   static – pre-built JSON files + Compiler Explorer (the GitHub Pages build)

import { remoteRun, diffOutput, normalizeOutput, makeQuiz, makeMatch } from './static-backend.js';

export const isStatic = document.querySelector('meta[name="app-mode"]')?.content === 'static';

const cache = new Map();

async function request(url, options = {}) {
  const res = await fetch(url, {
    headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
    ...options,
  });
  let data = null;
  try {
    data = await res.json();
  } catch {
    /* non-JSON body */
  }
  if (!res.ok) {
    const err = new Error((data && data.error) || `HTTP ${res.status}`);
    err.status = res.status;
    throw err;
  }
  return data;
}

function cached(url) {
  if (!cache.has(url)) {
    cache.set(url, request(url).catch((e) => {
      cache.delete(url);
      throw e;
    }));
  }
  return cache.get(url);
}

const id = (s) => encodeURIComponent(s);

const serverApi = {
  meta: () => cached('api/meta'),
  patterns: () => cached('api/patterns'),
  pattern: (p) => cached(`api/patterns/${id(p)}`),
  solution: (p) => request(`api/patterns/${id(p)}/solution`),
  run: (code) => request('api/run', { method: 'POST', body: JSON.stringify({ code }) }),
  check: (p, code) => request(`api/patterns/${id(p)}/check`, { method: 'POST', body: JSON.stringify({ code }) }),
  quiz: (params) => request(`api/training/quiz?${new URLSearchParams(params)}`),
  match: (params) => request(`api/training/match?${new URLSearchParams(params)}`),
};

const staticApi = {
  meta: () => cached('api/meta.json'),
  patterns: () => cached('api/patterns.json'),
  pattern: (p) => cached(`api/patterns/${id(p)}.json`).catch((e) => {
    // GitHub Pages answers unknown files with 404 HTML; keep the "not found" semantics.
    throw Object.assign(e, { status: 404 });
  }),
  solution: (p) => cached(`api/patterns/${id(p)}/solution.json`),
  run: (code) => remoteRun(code),
  async check(p, code) {
    const [lesson, result] = await Promise.all([staticApi.pattern(p), remoteRun(code)]);
    const expected = lesson.exercise.expectedOutput;
    const diff = result.stage === 'run' && !result.timedOut ? diffOutput(result.stdout, expected) : null;
    return { ...result, passed: result.ok && diff === null, diff, expected: normalizeOutput(expected) };
  },
  quiz: async (params) => makeQuiz(await cached('api/training.json'), params),
  match: async (params) => makeMatch(await cached('api/training.json'), params),
};

export const api = isStatic ? staticApi : serverApi;
