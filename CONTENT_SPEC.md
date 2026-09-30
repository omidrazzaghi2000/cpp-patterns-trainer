# Pattern Content Specification

Every pattern lives in `content/patterns/<id>/` and consists of four files:

| File | Purpose |
|------|---------|
| `pattern.json` | All bilingual teaching text, quiz, diagram, exercise metadata |
| `example.cpp` | A complete, runnable demonstration of the pattern |
| `exercise.cpp` | Starter code for the learner's coding challenge (compiles, but produces the wrong output) |
| `solution.cpp` | Reference solution of the exercise |

**The reference implementation is `content/patterns/singleton/` — match its depth, tone and format.**

Validate with:

```bash
node scripts/validate-content.js <id> [<id> ...]
```

A pattern is done only when it prints `[ OK ]` or `[WARN]` where the only warnings are
`related[..] does not exist (yet)` for ids from the master list below.

---

## Master list of pattern ids

| Category | ids in order (`order` field = position, starting at 1) |
|----------|-----|
| `creational` | `singleton`, `factory-method`, `abstract-factory`, `builder`, `prototype` |
| `structural` | `adapter`, `bridge`, `composite`, `decorator`, `facade`, `flyweight`, `proxy` |
| `behavioral` | `chain-of-responsibility`, `command`, `interpreter`, `iterator`, `mediator`, `memento`, `observer`, `state`, `strategy`, `template-method`, `visitor` |
| `idioms` | `raii`, `pimpl`, `crtp`, `type-erasure`, `policy-based-design`, `null-object`, `object-pool`, `copy-and-swap` |

`related[].id` must come from this list.

---

## C++ source rules (all three .cpp files)

- Single file, **standard library only**, must compile with
  `g++ -std=c++20 -Wall -Wextra -pedantic -Werror` (the validator uses exactly this).
- **Deterministic output**: no randomness, time, thread timing, pointer addresses, or `typeid().name()`.
  Programs read nothing from stdin and finish in well under a second.
- Modern, idiomatic C++17/20: `std::unique_ptr`/`std::make_unique`, `override`, `= delete`,
  `const`-correctness, range-for, `std::string_view` where sensible. No raw owning `new`/`delete`
  (except when the idiom itself is about managing a raw resource, e.g. RAII, copy-and-swap).
- Readable for learners: 40–90 lines for `example.cpp`, meaningful names, short comments that explain
  *why*, a realistic domain (text editor, payments, game, UI, file system, pizza shop…), 4-space indent,
  max ~95 columns per line.
- Start each file with a one-line comment: `// <Pattern>: <what this program shows>`.
- Output lines should be short and self-explanatory (they are shown in a console panel).
- `exercise.cpp` **must compile cleanly as given** and run, but its output must differ from
  `expectedOutput`. Mark the work with `// TODO 1:`, `// TODO 2:` … comments. Keep the required edits
  small and focused on the pattern (typically 5–25 lines of changes).
- `solution.cpp` is the starter with the TODOs completed (remove the TODO comments).

## pattern.json schema

All user-visible text is an object `{ "en": "...", "fa": "..." }`; lists are `{ "en": [..], "fa": [..] }`
with the **same number of items** in both languages.

Inline formatting allowed inside text: `` `inline code` `` and `**bold**`. Nothing else (no links, no
headings, no HTML). Use `\n` in JSON only inside `output` / `expectedOutput`.

```jsonc
{
  "id": "builder",                   // = folder name, kebab-case
  "category": "creational",          // creational | structural | behavioral | idioms
  "order": 4,                        // position in the master list
  "difficulty": 2,                   // 1 beginner, 2 intermediate, 3 advanced
  "icon": "hammer",                  // file name (without .svg) in node_modules/lucide-static/icons/
  "name":    { "en": "Builder", "fa": "بیلدر (سازنده)" },                     // <= 40 chars
  "aka":     { "en": "...", "fa": "..." },                                     // optional
  "tagline": { "en": "...", "fa": "..." },                                     // <= 90 chars, catchy
  "intent":  { "en": "...", "fa": "..." },                                     // 1–2 sentences
  "problem": { "en": "...", "fa": "..." },                                     // concrete paragraph
  "solution":{ "en": "...", "fa": "..." },                                     // paragraph
  "analogy": { "en": "...", "fa": "..." },                                     // real-world analogy
  "participants": [ { "name": "Builder", "role": { "en": "...", "fa": "..." } } ],
  "diagram": {
    "nodes": [
      { "id": "Director", "kind": "concrete", "col": 0, "row": 0, "members": ["+ construct()"] }
    ],
    "edges": [ { "from": "Director", "to": "Builder", "type": "refs", "label": "builder" } ]
  },
  "steps":      { "en": [3–7 items], "fa": [...] },   // how to implement it
  "whenToUse":  { "en": [2–6], "fa": [...] },
  "pros":       { "en": [2–6], "fa": [...] },
  "cons":       { "en": [1–5], "fa": [...] },
  "cppTips":    { "en": [2–6], "fa": [...] },          // modern-C++-specific advice & pitfalls
  "realWorld":  { "en": [1–5], "fa": [...] },          // std library / Qt / Boost / well-known code
  "related": [ { "id": "abstract-factory", "note": { "en": "how they differ / combine", "fa": "..." } } ],
  "example": {
    "title": {...}, "description": {...},
    "output": "exact stdout of example.cpp (\\n separated)",
    "highlights": [ { "lines": [start, end], "note": {...} } ]   // 3–5 walkthrough notes, 1-based lines
  },
  "quiz": [ 4 × { "question": {...}, "options": { "en": [4], "fa": [4] }, "answer": 0-3, "explanation": {...} } ],
  "scenarios": [ 2 × { "en": "...", "fa": "..." } ],
  "exercise": {
    "title": {...}, "task": {...},
    "hints": { "en": [2–4], "fa": [...] },           // progressive: vague → specific
    "expectedOutput": "exact stdout of solution.cpp"
  }
}
```

### Diagram (UML-style class diagram, rendered as clay blocks)

- Grid of **4 columns × 4 rows** (`col` 0–3, `row` 0–3), one node per cell. Put abstractions on top
  (row 0) and implementations below them; clients on the left (col 0). Leave empty cells rather than
  crowding. 3–7 nodes is ideal.
- `kind`: `client` (code using the pattern), `interface` (pure abstract), `abstract` (abstract class
  with some implementation), `concrete`, `note`.
- `id` is also the displayed title (max ~22 chars, e.g. `ConcreteBuilder`). Optional `label` overrides
  the display text.
- `members` (optional, ≤ 4 entries, ≤ 30 chars each) in UML style: `+ build() : Product`,
  `- state_ : int`, `# step()`.
- Edge `type`:
  - `inherits` – class inheritance (solid line, hollow triangle at `to`)
  - `implements` – implements an interface (dashed line, hollow triangle at `to`)
  - `uses` – dependency / calls (dashed arrow)
  - `creates` – instantiates (dashed arrow, rendered with «create»)
  - `refs` – association: holds a pointer/reference (solid arrow)
  - `has` – aggregation: `from` has `to` (hollow diamond at `from`)
  - `owns` – composition: `from` owns `to` (filled diamond at `from`)
- Optional edge `label` ≤ 18 chars.

### Quiz

- Exactly **4** questions: at least one conceptual ("what problem does it solve"), one about the C++
  implementation (code-level detail), one on trade-offs / when not to use, one comparing with a
  related pattern.
- 4 plausible options each; **vary the index of the correct answer** across questions (0–3).
- The Persian options must be in the **same order** as the English ones (same `answer` index).
- Explanations teach — say *why* the right answer is right.

### Scenarios

Two short real-world situations (1–2 sentences) for which this pattern is the best fit. Used in the
"Pattern Match" game where the learner picks the pattern — **never mention the pattern's name** or an
obvious giveaway word from it.

## Persian (fa) writing guidelines

- Natural, fluent, technical Persian as written by an experienced Iranian developer — not a word-by-word
  translation. Use the zero-width non-joiner (ZWNJ, U+200C) correctly: `می‌شود`، `کلاس‌ها`، `پیاده‌سازی`.
- Keep established English technical terms in Latin script where Iranian developers do
  (e.g. `thread-safe`, `override`, `template`, `virtual`, `mock`), optionally with a Persian equivalent
  in parentheses the first time. Code identifiers always in backticks.
- Write "C++" as `C++` (the app fixes bidi direction automatically). Don't reverse it manually.
- Pattern `name.fa`: Persian transliteration + common Persian translation in parentheses when one
  exists, e.g. `"آداپتر (مبدل)"`, `"دکوراتور (تزئین‌گر)"`, `"آبزرور (ناظر)"`.
- Use Persian punctuation: `،` for commas, `؛` for semicolons, `«»` for quotes.
