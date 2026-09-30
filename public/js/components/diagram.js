// Renders a pattern's class diagram (content/…/pattern.json → diagram) as an
// inline SVG: clay-style blocks on a 4×4 grid with UML relationship markers.

import { esc } from '../util.js';

const CAT_COLORS = {
  creational: { main: '#7C3AED', soft: '#EDE9FE', strong: '#5B21B6' },
  structural: { main: '#0E7490', soft: '#CFFAFE', strong: '#155E75' },
  behavioral: { main: '#DB2777', soft: '#FCE7F3', strong: '#9D174D' },
  idioms: { main: '#D97706', soft: '#FEF3C7', strong: '#92400E' },
};
const INK = '#1E1B4B';
const MUTED = '#475569';

const HEAD_H = 44;
const STEREO_H = 14;
const MEMBER_H = 19;
const ROW_GAP = 78;
const COL_GAP = 48;
const MARGIN = 18;
const NAME_CHAR_W = 8.6;
const MEMBER_CHAR_W = 6.7;
const DASHED = new Set(['implements', 'uses', 'creates']);
const STEREOTYPE = { interface: '«interface»', abstract: '«abstract»', client: '«client»', note: '' };

let uid = 0;

export function renderDiagram(diagram, { category = 'creational', title = '', legendLabels = null } = {}) {
  const id = `dg${++uid}`;
  const col = CAT_COLORS[category] || CAT_COLORS.creational;
  const nodes = diagram.nodes.map((n) => ({ ...n, members: n.members || [], label: n.label || n.id }));
  if (!nodes.length) return '';

  // Trim unused leading rows/cols so authors can place freely.
  const minCol = Math.min(...nodes.map((n) => n.col));
  const minRow = Math.min(...nodes.map((n) => n.row));
  nodes.forEach((n) => { n.c = n.col - minCol; n.r = n.row - minRow; });
  const cols = Math.max(...nodes.map((n) => n.c)) + 1;
  const rows = Math.max(...nodes.map((n) => n.r)) + 1;

  // Size nodes from their content.
  for (const n of nodes) {
    const stereo = STEREOTYPE[n.kind] ? STEREO_H : 0;
    n.h = HEAD_H + stereo + (n.members.length ? n.members.length * MEMBER_H + 14 : 0);
    const nameW = n.label.length * NAME_CHAR_W + 36;
    const memW = Math.max(0, ...n.members.map((m) => m.length * MEMBER_CHAR_W + 28));
    n.w = Math.max(150, nameW, memW);
  }
  const colW = Math.max(...nodes.map((n) => n.w)) + COL_GAP;
  const rowH = Array.from({ length: rows }, (_, r) => Math.max(48, ...nodes.filter((n) => n.r === r).map((n) => n.h)));
  const rowY = [];
  rowH.reduce((y, h, r) => { rowY[r] = y; return y + h + ROW_GAP; }, MARGIN);

  for (const n of nodes) {
    n.cx = MARGIN + n.c * colW + colW / 2;
    n.cy = rowY[n.r] + rowH[n.r] / 2;
    n.x = n.cx - n.w / 2;
    n.y = n.cy - n.h / 2;
  }
  const width = MARGIN * 2 + cols * colW;
  const height = rowY[rows - 1] + rowH[rows - 1] + MARGIN + 6;
  const byId = new Map(nodes.map((n) => [n.id, n]));

  // Parallel edges between the same pair get spread apart.
  const pairCount = new Map();
  const pairIndex = new Map();
  const pairKey = (e) => [e.from, e.to].sort().join('\u0000');
  diagram.edges.forEach((e) => pairCount.set(pairKey(e), (pairCount.get(pairKey(e)) || 0) + 1));

  const edgeSvg = [];
  const labelSvg = [];
  let minX = 0;
  let maxX = width;
  const extend = (x0, x1) => { minX = Math.min(minX, x0 - 6); maxX = Math.max(maxX, x1 + 6); };
  for (const e of diagram.edges) {
    const a = byId.get(e.from);
    const b = byId.get(e.to);
    if (!a || !b) continue;
    const key = pairKey(e);
    const k = pairCount.get(key);
    const i = pairIndex.get(key) || 0;
    pairIndex.set(key, i + 1);
    const isGeneralization = e.type === 'inherits' || e.type === 'implements';
    let pts;
    if (k > 1 && a.r !== b.r && !isGeneralization) {
      // e.g. Composite ◆→ Component next to Composite —▷ Component: loop around the side
      pts = sideRoute(a, b, i);
    } else if (k > 1 && a.r === b.r) {
      pts = route(a, b, (i - (k - 1) / 2) * 16);
    } else {
      pts = route(a, b, 0); // generalizations to one parent merge into a single triangle
    }
    pts.forEach(([x]) => extend(x, x));
    const d = `M${pts.map((p) => `${round(p[0])},${round(p[1])}`).join(' L')}`;
    const dash = DASHED.has(e.type) ? ' stroke-dasharray="7 6"' : '';
    const start = e.type === 'has' ? ` marker-start="url(#${id}-dia)"` : e.type === 'owns' ? ` marker-start="url(#${id}-diaf)"` : '';
    const end = e.type === 'inherits' || e.type === 'implements' ? ` marker-end="url(#${id}-tri)"` : e.type === 'has' || e.type === 'owns' ? ` marker-end="url(#${id}-arr)"` : ` marker-end="url(#${id}-arr)"`;
    edgeSvg.push(`<path d="${d}" fill="none" stroke="${INK}" stroke-width="2.2" stroke-linejoin="round"${dash}${start}${end}/>`);

    const text = e.label || (e.type === 'creates' ? '«create»' : '');
    if (text) {
      const [lx, ly] = midpoint(pts);
      const w = text.length * 6.9 + 16;
      extend(lx - w / 2, lx + w / 2);
      labelSvg.push(
        `<g transform="translate(${round(lx)},${round(ly)})"><rect x="${round(-w / 2)}" y="-11" width="${round(w)}" height="22" rx="11" fill="#FFFFFF" stroke="${col.main}" stroke-width="1.5"/>` +
        `<text y="4.5" text-anchor="middle" font-family="JetBrains Mono, monospace" font-size="11.5" font-weight="600" fill="${INK}">${esc(text)}</text></g>`,
      );
    }
  }

  const nodeSvg = nodes.map((n) => renderNode(n, col)).join('');

  const desc = diagram.edges
    .map((e) => `${byId.get(e.from)?.label ?? e.from} ${e.type} ${byId.get(e.to)?.label ?? e.to}`)
    .join('; ');

  const vbW = maxX - minX;
  return `<svg viewBox="${round(minX)} 0 ${round(vbW)} ${round(height)}" width="${round(vbW)}" height="${round(height)}" role="img" aria-labelledby="${id}-t ${id}-d" dir="ltr" xmlns="http://www.w3.org/2000/svg">
  <title id="${id}-t">${esc(title)}</title>
  <desc id="${id}-d">${esc(desc)}</desc>
  <defs>
    <marker id="${id}-tri" viewBox="0 0 16 16" refX="15" refY="8" markerWidth="16" markerHeight="16" markerUnits="userSpaceOnUse" orient="auto"><path d="M1,1 L15,8 L1,15 z" fill="#FFFFFF" stroke="${INK}" stroke-width="2" stroke-linejoin="round"/></marker>
    <marker id="${id}-arr" viewBox="0 0 14 14" refX="12" refY="7" markerWidth="14" markerHeight="14" markerUnits="userSpaceOnUse" orient="auto"><path d="M2,2 L12,7 L2,12" fill="none" stroke="${INK}" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/></marker>
    <marker id="${id}-dia" viewBox="0 0 20 12" refX="1" refY="6" markerWidth="20" markerHeight="12" markerUnits="userSpaceOnUse" orient="auto"><path d="M1,6 L10,1 L19,6 L10,11 z" fill="#FFFFFF" stroke="${INK}" stroke-width="1.8" stroke-linejoin="round"/></marker>
    <marker id="${id}-diaf" viewBox="0 0 20 12" refX="1" refY="6" markerWidth="20" markerHeight="12" markerUnits="userSpaceOnUse" orient="auto"><path d="M1,6 L10,1 L19,6 L10,11 z" fill="${INK}" stroke="${INK}" stroke-width="1.8" stroke-linejoin="round"/></marker>
  </defs>
  <g>${edgeSvg.join('')}</g>
  <g>${nodeSvg}</g>
  <g>${labelSvg.join('')}</g>
</svg>`;
}

function renderNode(n, col) {
  const palette = {
    interface: { fill: col.soft, stroke: col.main, depth: col.strong },
    abstract: { fill: col.soft, stroke: col.main, depth: col.strong },
    concrete: { fill: '#FFFFFF', stroke: col.main, depth: col.strong },
    client: { fill: '#E0E7FF', stroke: '#6366F1', depth: '#3730A3' },
    note: { fill: '#FEF3C7', stroke: '#D97706', depth: '#92400E' },
  }[n.kind] || { fill: '#FFFFFF', stroke: col.main, depth: col.strong };

  const italic = n.kind === 'interface' || n.kind === 'abstract' ? ' font-style="italic"' : '';
  const stereo = STEREOTYPE[n.kind];
  let y = n.y + (stereo ? 22 : 28);
  const parts = [];
  // clay depth + body + inner highlight
  parts.push(`<rect x="${round(n.x)}" y="${round(n.y + 5)}" width="${round(n.w)}" height="${round(n.h)}" rx="16" fill="${palette.depth}" opacity="0.35"/>`);
  parts.push(`<rect x="${round(n.x)}" y="${round(n.y)}" width="${round(n.w)}" height="${round(n.h)}" rx="16" fill="${palette.fill}" stroke="${palette.stroke}" stroke-width="3"${n.kind === 'interface' ? ' stroke-dasharray="0"' : ''}/>`);
  parts.push(`<rect x="${round(n.x + 8)}" y="${round(n.y + 5)}" width="${round(n.w - 16)}" height="8" rx="4" fill="#FFFFFF" opacity="0.55"/>`);
  if (stereo) {
    parts.push(`<text x="${round(n.cx)}" y="${round(y)}" text-anchor="middle" font-family="Nunito, sans-serif" font-size="11" font-weight="700" fill="${MUTED}">${esc(stereo)}</text>`);
    y += 18;
  }
  parts.push(`<text x="${round(n.cx)}" y="${round(y)}" text-anchor="middle" font-family="Fredoka, Nunito, sans-serif" font-size="15.5" font-weight="700" fill="${INK}"${italic}>${esc(n.label)}</text>`);
  if (n.members.length) {
    const lineY = n.y + HEAD_H + (stereo ? STEREO_H : 0);
    parts.push(`<line x1="${round(n.x + 10)}" x2="${round(n.x + n.w - 10)}" y1="${round(lineY)}" y2="${round(lineY)}" stroke="${palette.stroke}" stroke-opacity="0.45" stroke-width="2" stroke-dasharray="2 4" stroke-linecap="round"/>`);
    n.members.forEach((m, i) => {
      parts.push(`<text x="${round(n.x + 14)}" y="${round(lineY + 20 + i * MEMBER_H)}" font-family="JetBrains Mono, monospace" font-size="11.5" fill="${INK}">${esc(m)}</text>`);
    });
  }
  return `<g>${parts.join('')}</g>`;
}

/** Orthogonal route between two nodes; returns a polyline as [[x,y],…]. */
function route(a, b, offset) {
  if (a.r === b.r) {
    const dir = b.cx > a.cx ? 1 : -1;
    const y = a.cy + offset;
    return [[a.cx + (dir * a.w) / 2, y], [b.cx - (dir * b.w) / 2, y]];
  }
  const down = b.r > a.r;
  const sx = a.cx + offset;
  const tx = b.cx + offset;
  const sy = down ? a.y + a.h : a.y;
  const ty = down ? b.y : b.y + b.h + 4; // +4 leaves room for the clay depth under the target
  if (Math.abs(sx - tx) < 2) return [[sx, sy], [tx, ty]];
  // Bend in the gap next to the target, so edges into one node share a trunk (UML tree look).
  const midY = down ? b.y - ROW_GAP / 2 : b.y + b.h + ROW_GAP / 2;
  return [[sx, sy], [sx, midY], [tx, midY], [tx, ty]];
}

/** Route that leaves and enters on the outer side of both nodes (for parallel non-inheritance edges). */
function sideRoute(a, b, n) {
  const right = a.cx >= b.cx;
  const dir = right ? 1 : -1;
  const sx = a.cx + (dir * a.w) / 2;
  const tx = b.cx + (dir * b.w) / 2;
  const outX = (right ? Math.max(sx, tx) : Math.min(sx, tx)) + dir * (26 + n * 14);
  return [[sx, a.cy], [outX, a.cy], [outX, b.cy], [tx, b.cy]];
}

function midpoint(pts) {
  const segs = [];
  let total = 0;
  for (let i = 1; i < pts.length; i++) {
    const len = Math.hypot(pts[i][0] - pts[i - 1][0], pts[i][1] - pts[i - 1][1]);
    segs.push(len);
    total += len;
  }
  let half = total / 2;
  for (let i = 0; i < segs.length; i++) {
    if (half <= segs[i]) {
      const t = segs[i] ? half / segs[i] : 0;
      return [pts[i][0] + (pts[i + 1][0] - pts[i][0]) * t, pts[i][1] + (pts[i + 1][1] - pts[i][1]) * t];
    }
    half -= segs[i];
  }
  return pts[0];
}

function round(v) {
  return Math.round(v * 10) / 10;
}

/** Small inline legend swatches for each relationship type used by the diagram. */
export function renderLegend(diagram, labels) {
  const used = [...new Set(diagram.edges.map((e) => e.type))];
  return used
    .map((type) => {
      const dash = DASHED.has(type) ? ' stroke-dasharray="5 4"' : '';
      let marker = '';
      if (type === 'inherits' || type === 'implements') marker = '<path d="M32,2 L42,8 L32,14 z" fill="#fff" stroke="#1E1B4B" stroke-width="1.6"/>';
      else if (type === 'has' || type === 'owns') marker = `<path d="M2,8 L9,4 L16,8 L9,12 z" fill="${type === 'owns' ? '#1E1B4B' : '#fff'}" stroke="#1E1B4B" stroke-width="1.5"/><path d="M34,3 L41,8 L34,13" fill="none" stroke="#1E1B4B" stroke-width="1.8"/>`;
      else marker = '<path d="M34,3 L41,8 L34,13" fill="none" stroke="#1E1B4B" stroke-width="1.8"/>';
      const x1 = type === 'has' || type === 'owns' ? 16 : 2;
      const x2 = type === 'inherits' || type === 'implements' ? 32 : 40;
      return `<span><svg viewBox="0 0 44 16" aria-hidden="true"><line x1="${x1}" y1="8" x2="${x2}" y2="8" stroke="#1E1B4B" stroke-width="2"${dash}/>${marker}</svg>${esc(labels[type] || type)}</span>`;
    })
    .join('');
}
