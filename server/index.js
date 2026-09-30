'use strict';

const { createApp } = require('./app');
const { loadLibrary } = require('./content');
const { compilerInfo } = require('./runner');

// Bound to localhost by default: the /api/run endpoint compiles and executes
// arbitrary C++ and must not be reachable from other machines.
const HOST = process.env.HOST || '127.0.0.1';
const PORT = Number(process.env.PORT) || 3000;

const library = loadLibrary();
for (const { id, errors, warnings } of library.problems) {
  for (const e of errors) console.warn(`[content] ${id}: ${e}`);
  for (const w of warnings) console.warn(`[content] ${id}: ${w}`);
}

const app = createApp({ library });

app.listen(PORT, HOST, async () => {
  const info = await compilerInfo();
  console.log(`\n  C++ Patterns Trainer  →  http://${HOST === '0.0.0.0' ? 'localhost' : HOST}:${PORT}\n`);
  console.log(`  ${library.patterns.length} patterns loaded`);
  console.log(info.available
    ? `  compiler: ${info.version} (-std=${info.standard})`
    : '  compiler: not found — code running is disabled (install g++ or set CXX)');
  if (HOST !== '127.0.0.1' && HOST !== 'localhost') {
    console.warn('\n  WARNING: listening on a non-local interface. Anyone who can reach this port can run C++ code on this machine.');
  }
});
