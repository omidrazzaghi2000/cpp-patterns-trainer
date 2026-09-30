'use strict';

// Builds a single SVG sprite from lucide-static containing only the icons the
// app uses (UI icons + every category/pattern icon), so the browser downloads
// one small file instead of the full 2,000-icon set.

const fs = require('fs');
const path = require('path');
const { ICONS_DIR } = require('./content');

const UI_ICONS = [
  'house', 'layout-grid', 'dumbbell', 'terminal', 'trophy', 'languages', 'search', 'x', 'check',
  'circle-check', 'circle-x', 'chevron-left', 'chevron-right', 'chevron-down', 'arrow-left',
  'arrow-right', 'play', 'copy', 'rotate-ccw', 'lightbulb', 'eye', 'eye-off', 'book-open',
  'graduation-cap', 'flame', 'star', 'zap', 'target', 'sparkles', 'code-xml', 'network',
  'list-checks', 'puzzle', 'clock', 'award', 'medal', 'lock', 'lock-open', 'download', 'upload',
  'trash-2', 'info', 'triangle-alert', 'loader-circle', 'shuffle', 'brain', 'timer', 'gauge',
  'circle-help', 'check-check', 'bookmark', 'bookmark-check', 'arrow-up-right', 'user',
  'list-ordered', 'file-code', 'blocks', 'swords', 'repeat-2', 'party-popper', 'route', 'map',
  'footprints', 'keyboard', 'wand-sparkles', 'square-terminal', 'git-compare', 'heart',
];

function symbolFor(name) {
  const file = path.join(ICONS_DIR, `${name}.svg`);
  if (!fs.existsSync(file)) return null;
  const inner = fs
    .readFileSync(file, 'utf8')
    .replace(/<!--[\s\S]*?-->/g, '')
    .replace(/^[\s\S]*?<svg[^>]*>/, '')
    .replace(/<\/svg>\s*$/, '')
    .replace(/\s*\n\s*/g, '')
    .trim();
  // stroke-width is left to CSS (.icon) so it can be tuned per context.
  return `<symbol id="i-${name}" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round">${inner}</g></symbol>`;
}

function buildSprite(extraNames = []) {
  const names = [...new Set([...UI_ICONS, ...extraNames])].sort();
  const symbols = names.map(symbolFor).filter(Boolean);
  return `<svg xmlns="http://www.w3.org/2000/svg">${symbols.join('')}</svg>`;
}

module.exports = { buildSprite, UI_ICONS };
