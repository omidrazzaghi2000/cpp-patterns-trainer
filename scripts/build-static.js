#!/usr/bin/env node
'use strict';

// Builds a fully static copy of the trainer into dist/ (for GitHub Pages or any
// static host). Lesson data is pre-rendered to JSON, vendor files are copied
// from node_modules, and the page switches to "static" mode, in which C++ is
// compiled by Compiler Explorer (godbolt.org) instead of the local server.
//
//   npm run build:static     → dist/
//   npm run preview:static   → serves dist/ on http://127.0.0.1:4000

const fs = require('fs');
const path = require('path');
const content = require('../server/content');
const { buildSprite } = require('../server/icons');

const ROOT = path.join(__dirname, '..');
const OUT = path.join(ROOT, 'dist');
const MODULES = path.join(ROOT, 'node_modules');

const REMOTE_COMPILER = {
  available: true,
  remote: true,
  command: 'g++ 13.4',
  standard: 'c++20',
  version: 'x86-64 gcc 13.4 · Compiler Explorer (godbolt.org)',
};

const CSP = [
  "default-src 'self'",
  "script-src 'self'",
  "style-src 'self' 'unsafe-inline'",
  "img-src 'self' data:",
  "font-src 'self'",
  "connect-src 'self' https://godbolt.org",
  "base-uri 'self'",
  "form-action 'self'",
].join('; ');

function copy(src, dest) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  fs.cpSync(src, dest, { recursive: true });
}

function writeJson(rel, data) {
  const file = path.join(OUT, rel);
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, JSON.stringify(data));
}

function main() {
  const library = content.loadLibrary();
  const errors = library.problems.filter((p) => p.errors.length);
  if (errors.length) {
    for (const p of errors) console.error(`${p.id}: ${p.errors.join('; ')}`);
    process.exit(1);
  }
  const { categories, patterns } = library;

  fs.rmSync(OUT, { recursive: true, force: true });
  copy(path.join(ROOT, 'public'), OUT);

  // index.html → static mode + CSP (GitHub Pages cannot send headers)
  const indexFile = path.join(OUT, 'index.html');
  let html = fs.readFileSync(indexFile, 'utf8');
  if (!html.includes('<meta name="app-mode" content="server">')) throw new Error('index.html is missing the app-mode meta tag');
  html = html.replace('<meta name="app-mode" content="server">', `<meta name="app-mode" content="static">\n  <meta http-equiv="Content-Security-Policy" content="${CSP}">`);
  fs.writeFileSync(indexFile, html);
  fs.writeFileSync(path.join(OUT, '.nojekyll'), '');

  // CodeMirror: only the files index.html loads
  const cm = path.join(MODULES, 'codemirror');
  for (const rel of ['lib/codemirror.js', 'lib/codemirror.css', 'mode/clike/clike.js', 'addon/edit/matchbrackets.js', 'addon/edit/closebrackets.js', 'addon/selection/active-line.js', 'LICENSE']) {
    copy(path.join(cm, rel), path.join(OUT, 'vendor/codemirror', rel));
  }

  // Fonts: each CSS file imported by public/css/fonts.css, plus the font files it references
  const fontsCss = fs.readFileSync(path.join(ROOT, 'public/css/fonts.css'), 'utf8');
  let fontFiles = 0;
  for (const [, rel] of fontsCss.matchAll(/url\('\.\.\/vendor\/fonts\/([^']+)'\)/g)) {
    const src = path.join(MODULES, '@fontsource', rel);
    const css = fs.readFileSync(src, 'utf8');
    copy(src, path.join(OUT, 'vendor/fonts', rel));
    for (const [, file] of css.matchAll(/url\(\.\/(files\/[^)]+)\)/g)) {
      const from = path.join(path.dirname(src), file);
      const to = path.join(OUT, 'vendor/fonts', path.dirname(rel), file);
      if (!fs.existsSync(to)) {
        copy(from, to);
        fontFiles++;
      }
    }
    const pkg = rel.split('/')[0];
    const license = path.join(MODULES, '@fontsource', pkg, 'LICENSE');
    if (fs.existsSync(license)) copy(license, path.join(OUT, 'vendor/fonts', pkg, 'LICENSE'));
  }

  fs.writeFileSync(path.join(OUT, 'icons.svg'), buildSprite([...categories.map((c) => c.icon), ...patterns.map((p) => p.icon)]));

  // Pre-rendered API
  writeJson('api/meta.json', {
    categories,
    patternCount: patterns.length,
    questionCount: patterns.reduce((n, p) => n + p.quiz.length, 0),
    exerciseCount: patterns.filter((p) => p.exercise).length,
    compiler: REMOTE_COMPILER,
  });
  writeJson('api/patterns.json', patterns.map(content.summarize));
  for (const p of patterns) {
    writeJson(`api/patterns/${p.id}.json`, content.lessonPayload(p, library));
    writeJson(`api/patterns/${p.id}/solution.json`, { code: p.sources.solution });
  }
  writeJson('api/training.json', patterns.map((p) => ({ pattern: content.summarize(p), quiz: p.quiz, scenarios: p.scenarios })));

  const size = dirSize(OUT);
  console.log(`dist/ built: ${patterns.length} patterns, ${fontFiles} font files, ${(size / 1024 / 1024).toFixed(1)} MB`);
}

function dirSize(dir) {
  return fs.readdirSync(dir, { withFileTypes: true }).reduce((n, e) => {
    const p = path.join(dir, e.name);
    return n + (e.isDirectory() ? dirSize(p) : fs.statSync(p).size);
  }, 0);
}

main();
