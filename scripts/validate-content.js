#!/usr/bin/env node
'use strict';

// Validates pattern content.
//   node scripts/validate-content.js              # every pattern
//   node scripts/validate-content.js builder visitor
//   node scripts/validate-content.js --no-compile  # schema only
//
// Besides the JSON schema, it checks that example.cpp / exercise.cpp / solution.cpp
// compile warning-free, that example.cpp prints exactly example.output, that
// solution.cpp prints exactly exercise.expectedOutput, and that the untouched
// starter does NOT already pass the exercise.

const { listPatternIds, loadPattern, validatePattern } = require('../server/content');
const { compileAndRun, compilerInfo, diffOutput } = require('../server/runner');

const args = process.argv.slice(2);
const noCompile = args.includes('--no-compile');
const requested = args.filter((a) => !a.startsWith('--'));

const STRICT = ['-Werror', '-pedantic'];

async function checkProgram(label, source, expected, { mustMatch = true } = {}) {
  const r = await compileAndRun(source, { extraFlags: STRICT });
  if (r.stage === 'compile') return [`${label}: does not compile cleanly (-Wall -Wextra -pedantic -Werror):\n${indent(r.compileOutput)}`];
  if (r.timedOut) return [`${label}: timed out`];
  if (r.exitCode !== 0) return [`${label}: exited with code ${r.exitCode}${r.stderr ? `\n${indent(r.stderr)}` : ''}`];
  const diff = diffOutput(r.stdout, expected);
  if (mustMatch && diff) {
    return [`${label}: output differs at line ${diff.line}\n    expected: ${JSON.stringify(diff.expected)}\n    actual:   ${JSON.stringify(diff.actual)}\n  full actual output:\n${indent(r.stdout)}`];
  }
  if (!mustMatch && !diff) return [`${label}: the untouched starter already prints the expected output — the exercise needs work to pass`];
  return [];
}

function indent(s) {
  return String(s).trimEnd().split('\n').map((l) => `      ${l}`).join('\n');
}

(async () => {
  const all = listPatternIds();
  const ids = requested.length ? requested : all;
  const known = new Set(all);
  const missing = ids.filter((id) => !known.has(id));
  if (missing.length) {
    console.error(`Unknown pattern id(s): ${missing.join(', ')}`);
    process.exit(2);
  }
  if (!noCompile) {
    const info = await compilerInfo();
    if (!info.available) {
      console.error('No C++ compiler found; re-run with --no-compile to check the schema only.');
      process.exit(2);
    }
    console.log(`Compiler: ${info.version} (-std=${info.standard})`);
  }

  let failed = 0;
  for (const id of ids) {
    let p;
    const errors = [];
    let warnings = [];
    try {
      p = loadPattern(id);
      const v = validatePattern(p, { knownIds: known });
      errors.push(...v.errors);
      warnings = v.warnings;
    } catch (e) {
      errors.push(`cannot load: ${e.message}`);
    }
    if (p && !noCompile && p.sources) {
      if (p.sources.example && p.example) errors.push(...(await checkProgram('example.cpp', p.sources.example, p.example.output)));
      if (p.sources.solution && p.exercise) errors.push(...(await checkProgram('solution.cpp', p.sources.solution, p.exercise.expectedOutput)));
      if (p.sources.exercise && p.exercise) errors.push(...(await checkProgram('exercise.cpp', p.sources.exercise, p.exercise.expectedOutput, { mustMatch: false })));
    }
    if (errors.length) failed++;
    const status = errors.length ? 'FAIL' : warnings.length ? 'WARN' : ' OK ';
    console.log(`[${status}] ${id}`);
    for (const e of errors) console.log(`   ✗ ${e}`);
    for (const w of warnings) console.log(`   ! ${w}`);
  }
  console.log(`\n${ids.length - failed}/${ids.length} patterns valid.`);
  process.exit(failed ? 1 : 0);
})();
