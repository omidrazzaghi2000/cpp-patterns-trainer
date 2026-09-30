'use strict';

// Compiles and runs a single-file C++ program in a throw-away temp directory.
// Intended for a trainer running on the learner's own machine: jobs are
// time-limited, output-limited, memory-limited (on POSIX) and serialized
// through a small concurrency gate. Do not expose this to untrusted networks.

const { spawn } = require('child_process');
const crypto = require('crypto');
const fs = require('fs/promises');
const os = require('os');
const path = require('path');

const CXX = process.env.CXX || 'g++';
const CXX_STD = process.env.CXX_STD || 'c++20';
const COMPILE_TIMEOUT_MS = 30_000;
const RUN_TIMEOUT_MS = 5_000;
const MAX_OUTPUT_BYTES = 64 * 1024;
const MAX_SOURCE_BYTES = 100 * 1024;
const MAX_CONCURRENT_JOBS = 2;
const IS_POSIX = process.platform !== 'win32';

let active = 0;
const waiting = [];

async function withSlot(fn) {
  if (active >= MAX_CONCURRENT_JOBS) {
    await new Promise((resolve) => waiting.push(resolve));
  }
  active++;
  try {
    return await fn();
  } finally {
    active--;
    const next = waiting.shift();
    if (next) next();
  }
}

function exec(cmd, args, { cwd, timeoutMs }) {
  return new Promise((resolve) => {
    const started = Date.now();
    let stdout = '';
    let stderr = '';
    let bytes = 0;
    let truncated = false;
    let timedOut = false;

    let child;
    try {
      child = spawn(cmd, args, { cwd, stdio: ['ignore', 'pipe', 'pipe'] });
    } catch (err) {
      resolve({ code: -1, signal: null, stdout: '', stderr: String(err), timedOut, truncated, ms: 0, spawnError: true });
      return;
    }

    const collect = (which) => (chunk) => {
      if (truncated) return;
      bytes += chunk.length;
      if (bytes > MAX_OUTPUT_BYTES) {
        truncated = true;
        child.kill('SIGKILL');
        return;
      }
      if (which === 'out') stdout += chunk.toString('utf8');
      else stderr += chunk.toString('utf8');
    };
    child.stdout.on('data', collect('out'));
    child.stderr.on('data', collect('err'));

    const timer = setTimeout(() => {
      timedOut = true;
      child.kill('SIGKILL');
    }, timeoutMs);

    child.on('error', (err) => {
      clearTimeout(timer);
      resolve({ code: -1, signal: null, stdout, stderr: stderr + String(err), timedOut, truncated, ms: Date.now() - started, spawnError: true });
    });
    child.on('close', (code, signal) => {
      clearTimeout(timer);
      resolve({ code, signal, stdout, stderr, timedOut, truncated, ms: Date.now() - started });
    });
  });
}

let compilerInfoPromise = null;

function compilerInfo() {
  if (!compilerInfoPromise) {
    compilerInfoPromise = exec(CXX, ['--version'], { cwd: os.tmpdir(), timeoutMs: 5_000 }).then((r) => ({
      available: r.code === 0,
      command: CXX,
      standard: CXX_STD,
      version: r.code === 0 ? r.stdout.split('\n')[0].trim() : null,
    }));
  }
  return compilerInfoPromise;
}

/**
 * @param {string} source  C++ source code
 * @param {{ extraFlags?: string[] }} [opts]
 * @returns {Promise<{ok:boolean, stage:'compile'|'run', compileOutput:string, stdout:string,
 *   stderr:string, exitCode:number|null, timedOut:boolean, truncated:boolean, compileMs:number, runMs:number}>}
 */
async function compileAndRun(source, opts = {}) {
  if (typeof source !== 'string' || !source.trim()) {
    throw Object.assign(new Error('Source code is empty.'), { status: 400 });
  }
  if (Buffer.byteLength(source, 'utf8') > MAX_SOURCE_BYTES) {
    throw Object.assign(new Error('Source code is too large (limit 100 KB).'), { status: 413 });
  }
  const info = await compilerInfo();
  if (!info.available) {
    throw Object.assign(new Error(`C++ compiler "${CXX}" was not found on this machine.`), { status: 503 });
  }

  return withSlot(async () => {
    const dir = await fs.mkdtemp(path.join(os.tmpdir(), 'cppdp-'));
    try {
      const src = path.join(dir, 'main.cpp');
      const bin = path.join(dir, IS_POSIX ? 'main' : 'main.exe');
      await fs.writeFile(src, source, 'utf8');

      const flags = [`-std=${CXX_STD}`, '-O0', '-pipe', '-Wall', '-Wextra', ...(opts.extraFlags || [])];
      const compile = await exec(CXX, [...flags, 'main.cpp', '-o', bin], { cwd: dir, timeoutMs: COMPILE_TIMEOUT_MS });
      const compileOutput = cleanPaths(compile.stderr + compile.stdout, dir);

      if (compile.code !== 0) {
        return {
          ok: false, stage: 'compile', compileOutput: compile.timedOut ? 'Compilation timed out.' : compileOutput,
          stdout: '', stderr: '', exitCode: compile.code, timedOut: compile.timedOut, truncated: compile.truncated,
          compileMs: compile.ms, runMs: 0,
        };
      }

      // On POSIX, cap address space (512 MB) and CPU seconds via the shell's ulimit.
      // The CPU cap is only a backstop; the wall-clock timer normally fires first.
      const run = IS_POSIX
        ? await exec('/bin/sh', ['-c', 'ulimit -v 524288 2>/dev/null; ulimit -t 10 2>/dev/null; exec ./main'], { cwd: dir, timeoutMs: RUN_TIMEOUT_MS })
        : await exec(bin, [], { cwd: dir, timeoutMs: RUN_TIMEOUT_MS });
      if (run.signal === 'SIGXCPU') run.timedOut = true;

      return {
        ok: run.code === 0 && !run.timedOut && !run.truncated,
        stage: 'run', compileOutput, stdout: run.stdout, stderr: cleanPaths(run.stderr, dir),
        exitCode: run.code, signal: run.signal, timedOut: run.timedOut, truncated: run.truncated,
        compileMs: compile.ms, runMs: run.ms,
      };
    } finally {
      fs.rm(dir, { recursive: true, force: true }).catch(() => {});
    }
  });
}

function cleanPaths(text, dir) {
  return text.split(dir + path.sep).join('').split(dir).join('.');
}

/** Normalizes program output for comparison: unify newlines, strip trailing spaces and trailing blank lines. */
function normalizeOutput(text) {
  return String(text ?? '')
    .replace(/\r\n?/g, '\n')
    .split('\n')
    .map((line) => line.replace(/\s+$/, ''))
    .join('\n')
    .replace(/\n+$/, '');
}

/** Returns null when outputs match, otherwise the first differing line (1-based) with both versions. */
function diffOutput(actual, expected) {
  const a = normalizeOutput(actual).split('\n');
  const e = normalizeOutput(expected).split('\n');
  const n = Math.max(a.length, e.length);
  for (let i = 0; i < n; i++) {
    if (a[i] !== e[i]) {
      return { line: i + 1, expected: e[i] ?? null, actual: a[i] ?? null };
    }
  }
  return null;
}

function hashSource(source) {
  return crypto.createHash('sha256').update(source).digest('hex');
}

module.exports = { compileAndRun, compilerInfo, normalizeOutput, diffOutput, hashSource, CXX_STD };
