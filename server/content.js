'use strict';

// Loads the pattern library from content/ and validates it against the schema
// documented in CONTENT_SPEC.md.

const fs = require('fs');
const path = require('path');

const ROOT = path.join(__dirname, '..');
const CONTENT_DIR = path.join(ROOT, 'content');
const PATTERNS_DIR = path.join(CONTENT_DIR, 'patterns');
const ICONS_DIR = path.join(ROOT, 'node_modules', 'lucide-static', 'icons');

const CATEGORY_IDS = ['creational', 'structural', 'behavioral', 'idioms'];
const NODE_KINDS = ['client', 'interface', 'abstract', 'concrete', 'note'];
const EDGE_TYPES = ['inherits', 'implements', 'uses', 'creates', 'refs', 'has', 'owns'];
const LANGS = ['en', 'fa'];
const PERSIAN_RE = /[؀-ۿ]/;

function readJson(file) {
  return JSON.parse(fs.readFileSync(file, 'utf8'));
}

function loadCategories() {
  return readJson(path.join(CONTENT_DIR, 'categories.json')).sort((a, b) => a.order - b.order);
}

function listPatternIds() {
  if (!fs.existsSync(PATTERNS_DIR)) return [];
  return fs
    .readdirSync(PATTERNS_DIR, { withFileTypes: true })
    .filter((d) => d.isDirectory() && fs.existsSync(path.join(PATTERNS_DIR, d.name, 'pattern.json')))
    .map((d) => d.name);
}

function readSource(id, file) {
  const p = path.join(PATTERNS_DIR, id, file);
  return fs.existsSync(p) ? fs.readFileSync(p, 'utf8') : null;
}

function loadPattern(id) {
  const data = readJson(path.join(PATTERNS_DIR, id, 'pattern.json'));
  return {
    ...data,
    sources: {
      example: readSource(id, 'example.cpp'),
      exercise: readSource(id, 'exercise.cpp'),
      solution: readSource(id, 'solution.cpp'),
    },
  };
}

/** Loads every pattern; returns { categories, patterns (ordered array), byId (Map), problems }. */
function loadLibrary() {
  const categories = loadCategories();
  const catOrder = Object.fromEntries(categories.map((c) => [c.id, c.order]));
  const problems = [];
  const patterns = [];
  for (const id of listPatternIds()) {
    try {
      patterns.push(loadPattern(id));
    } catch (err) {
      problems.push({ id, errors: [`failed to load: ${err.message}`], warnings: [] });
    }
  }
  patterns.sort((a, b) => (catOrder[a.category] ?? 99) - (catOrder[b.category] ?? 99) || (a.order ?? 99) - (b.order ?? 99) || a.id.localeCompare(b.id));
  const byId = new Map(patterns.map((p) => [p.id, p]));
  for (const p of patterns) {
    const { errors, warnings } = validatePattern(p, { knownIds: new Set(byId.keys()) });
    if (errors.length || warnings.length) problems.push({ id: p.id, errors, warnings });
  }
  return { categories, patterns, byId, problems };
}

function summarize(p) {
  return {
    id: p.id,
    category: p.category,
    order: p.order,
    difficulty: p.difficulty,
    icon: p.icon,
    name: p.name,
    tagline: p.tagline,
    intent: p.intent,
    quizCount: Array.isArray(p.quiz) ? p.quiz.length : 0,
    exerciseTitle: p.exercise ? p.exercise.title : null,
  };
}

/** The JSON a lesson page receives: content + example code + exercise starter (never the solution). */
function lessonPayload(p, { patterns, byId }) {
  const idx = patterns.indexOf(p);
  const { sources, ...rest } = p;
  return {
    ...rest,
    example: { ...p.example, code: sources.example },
    exercise: { ...p.exercise, starter: sources.exercise },
    prev: idx > 0 ? summarize(patterns[idx - 1]) : null,
    next: idx < patterns.length - 1 ? summarize(patterns[idx + 1]) : null,
    related: (p.related || []).map((r) => ({ ...r, pattern: byId.has(r.id) ? summarize(byId.get(r.id)) : null })),
  };
}

// ---------------------------------------------------------------- validation

function validatePattern(p, { knownIds = null } = {}) {
  const errors = [];
  const warnings = [];
  const err = (m) => errors.push(m);
  const warn = (m) => warnings.push(m);

  const text = (value, where, { max } = {}) => {
    if (!value || typeof value !== 'object') return err(`${where}: expected { en, fa } object`);
    for (const lang of LANGS) {
      if (typeof value[lang] !== 'string' || !value[lang].trim()) err(`${where}.${lang}: missing text`);
      else if (max && value[lang].length > max) warn(`${where}.${lang}: longer than ${max} chars`);
    }
    if (typeof value.fa === 'string' && value.fa.trim() && !PERSIAN_RE.test(value.fa)) err(`${where}.fa: contains no Persian text`);
  };

  const list = (value, where, { min = 1, max = 8 } = {}) => {
    if (!value || typeof value !== 'object') return err(`${where}: expected { en: [], fa: [] }`);
    for (const lang of LANGS) {
      const arr = value[lang];
      if (!Array.isArray(arr)) { err(`${where}.${lang}: expected array`); continue; }
      if (arr.length < min || arr.length > max) err(`${where}.${lang}: expected ${min}-${max} items, got ${arr.length}`);
      arr.forEach((s, i) => { if (typeof s !== 'string' || !s.trim()) err(`${where}.${lang}[${i}]: empty`); });
    }
    if (Array.isArray(value.en) && Array.isArray(value.fa) && value.en.length !== value.fa.length) {
      err(`${where}: en has ${value.en.length} items but fa has ${value.fa.length}`);
    }
    if (Array.isArray(value.fa) && value.fa.some((s) => typeof s === 'string' && !PERSIAN_RE.test(s))) err(`${where}.fa: an item contains no Persian text`);
  };

  if (typeof p.id !== 'string' || !/^[a-z0-9]+(-[a-z0-9]+)*$/.test(p.id)) err('id: must be kebab-case');
  if (!CATEGORY_IDS.includes(p.category)) err(`category: must be one of ${CATEGORY_IDS.join(', ')}`);
  if (!Number.isInteger(p.order) || p.order < 1) err('order: positive integer required');
  if (![1, 2, 3].includes(p.difficulty)) err('difficulty: must be 1, 2 or 3');
  if (typeof p.icon !== 'string' || !fs.existsSync(path.join(ICONS_DIR, `${p.icon}.svg`))) err(`icon: "${p.icon}" is not a lucide-static icon`);

  text(p.name, 'name', { max: 40 });
  if (p.aka) text(p.aka, 'aka');
  text(p.tagline, 'tagline', { max: 90 });
  text(p.intent, 'intent', { max: 320 });
  text(p.problem, 'problem');
  text(p.solution, 'solution');
  text(p.analogy, 'analogy');
  list(p.steps, 'steps', { min: 3, max: 7 });
  list(p.whenToUse, 'whenToUse', { min: 2, max: 6 });
  list(p.pros, 'pros', { min: 2, max: 6 });
  list(p.cons, 'cons', { min: 1, max: 5 });
  list(p.cppTips, 'cppTips', { min: 2, max: 6 });
  list(p.realWorld, 'realWorld', { min: 1, max: 5 });

  // participants
  if (!Array.isArray(p.participants) || p.participants.length < 1) err('participants: at least one required');
  else p.participants.forEach((pt, i) => {
    if (typeof pt.name !== 'string' || !pt.name.trim()) err(`participants[${i}].name: missing`);
    text(pt.role, `participants[${i}].role`);
  });

  // diagram
  const d = p.diagram;
  if (!d || !Array.isArray(d.nodes) || !Array.isArray(d.edges)) err('diagram: { nodes: [], edges: [] } required');
  else {
    const ids = new Set();
    const cells = new Set();
    d.nodes.forEach((n, i) => {
      const w = `diagram.nodes[${i}]`;
      if (typeof n.id !== 'string' || !n.id) err(`${w}.id: missing`);
      else if (ids.has(n.id)) err(`${w}.id: duplicate "${n.id}"`);
      else ids.add(n.id);
      if (!NODE_KINDS.includes(n.kind)) err(`${w}.kind: must be one of ${NODE_KINDS.join(', ')}`);
      if (!Number.isInteger(n.col) || n.col < 0 || n.col > 3) err(`${w}.col: integer 0-3`);
      if (!Number.isInteger(n.row) || n.row < 0 || n.row > 3) err(`${w}.row: integer 0-3`);
      const cell = `${n.col},${n.row}`;
      if (cells.has(cell)) err(`${w}: cell (${cell}) already used by another node`);
      cells.add(cell);
      if (n.members !== undefined) {
        if (!Array.isArray(n.members) || n.members.length > 4) err(`${w}.members: array of at most 4 strings`);
        else n.members.forEach((m, j) => { if (typeof m !== 'string' || m.length > 30) err(`${w}.members[${j}]: string of at most 30 chars`); });
      }
      if (typeof n.id === 'string' && n.id.length > 22 && !n.label) warn(`${w}.id: long name may not fit (22 chars); add a shorter "label"`);
    });
    if (d.nodes.length < 2) err('diagram: at least 2 nodes');
    d.edges.forEach((e, i) => {
      const w = `diagram.edges[${i}]`;
      if (!ids.has(e.from)) err(`${w}.from: unknown node "${e.from}"`);
      if (!ids.has(e.to)) err(`${w}.to: unknown node "${e.to}"`);
      if (e.from === e.to) err(`${w}: self-edges are not supported`);
      if (!EDGE_TYPES.includes(e.type)) err(`${w}.type: must be one of ${EDGE_TYPES.join(', ')}`);
      if (e.label !== undefined && (typeof e.label !== 'string' || e.label.length > 18)) err(`${w}.label: string of at most 18 chars`);
    });
  }

  // related
  if (!Array.isArray(p.related) || p.related.length < 1) err('related: at least one entry');
  else p.related.forEach((r, i) => {
    if (typeof r.id !== 'string') err(`related[${i}].id: missing`);
    else if (knownIds && !knownIds.has(r.id)) warn(`related[${i}].id: "${r.id}" does not exist (yet)`);
    if (r.id === p.id) err(`related[${i}]: cannot relate to itself`);
    text(r.note, `related[${i}].note`);
  });

  // example
  const ex = p.example;
  if (!ex) err('example: required');
  else {
    text(ex.title, 'example.title');
    text(ex.description, 'example.description');
    if (typeof ex.output !== 'string' || !ex.output.trim()) err('example.output: expected program output required');
    const src = p.sources && p.sources.example;
    if (!src) err('example.cpp: file missing');
    const lineCount = src ? src.split('\n').length : 0;
    if (!Array.isArray(ex.highlights) || ex.highlights.length < 2) err('example.highlights: at least 2 entries');
    else ex.highlights.forEach((h, i) => {
      const w = `example.highlights[${i}]`;
      if (!Array.isArray(h.lines) || h.lines.length !== 2 || !h.lines.every(Number.isInteger) || h.lines[0] < 1 || h.lines[1] < h.lines[0]) err(`${w}.lines: [start, end] 1-based, start <= end`);
      else if (src && h.lines[1] > lineCount) err(`${w}.lines: end ${h.lines[1]} beyond file length ${lineCount}`);
      text(h.note, `${w}.note`);
    });
  }

  // quiz
  if (!Array.isArray(p.quiz) || p.quiz.length !== 4) err('quiz: exactly 4 questions required');
  else p.quiz.forEach((q, i) => {
    const w = `quiz[${i}]`;
    text(q.question, `${w}.question`);
    text(q.explanation, `${w}.explanation`);
    if (!q.options || !Array.isArray(q.options.en) || !Array.isArray(q.options.fa) || q.options.en.length !== 4 || q.options.fa.length !== 4) err(`${w}.options: { en: [4], fa: [4] } required`);
    else {
      [...q.options.en, ...q.options.fa].forEach((o, j) => { if (typeof o !== 'string' || !o.trim()) err(`${w}.options: option ${j} empty`); });
      if (new Set(q.options.en).size !== 4) err(`${w}.options.en: duplicate options`);
    }
    if (!Number.isInteger(q.answer) || q.answer < 0 || q.answer > 3) err(`${w}.answer: index 0-3`);
  });
  if (Array.isArray(p.quiz) && p.quiz.length === 4 && p.quiz.every((q) => q.answer === p.quiz[0].answer)) warn('quiz: every answer has the same index — vary the correct option position');

  // scenarios
  if (!Array.isArray(p.scenarios) || p.scenarios.length !== 2) err('scenarios: exactly 2 required');
  else p.scenarios.forEach((s, i) => {
    text(s, `scenarios[${i}]`);
    const nameEn = p.name && p.name.en ? p.name.en.toLowerCase() : null;
    if (nameEn && s && typeof s.en === 'string' && s.en.toLowerCase().includes(nameEn)) err(`scenarios[${i}].en: must not reveal the pattern name`);
  });

  // exercise
  const x = p.exercise;
  if (!x) err('exercise: required');
  else {
    text(x.title, 'exercise.title');
    text(x.task, 'exercise.task');
    list(x.hints, 'exercise.hints', { min: 2, max: 4 });
    if (typeof x.expectedOutput !== 'string' || !x.expectedOutput.trim()) err('exercise.expectedOutput: required');
    if (!p.sources || !p.sources.exercise) err('exercise.cpp: file missing');
    if (!p.sources || !p.sources.solution) err('solution.cpp: file missing');
  }

  return { errors, warnings };
}

function iconExists(name) {
  return fs.existsSync(path.join(ICONS_DIR, `${name}.svg`));
}

module.exports = {
  CONTENT_DIR, PATTERNS_DIR, ICONS_DIR, CATEGORY_IDS,
  loadCategories, listPatternIds, loadPattern, loadLibrary, summarize, lessonPayload, validatePattern, iconExists,
};
